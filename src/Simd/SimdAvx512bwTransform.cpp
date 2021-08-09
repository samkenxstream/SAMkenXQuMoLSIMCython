/*
* Simd Library (http://ermig1979.github.io/Simd).
*
* Copyright (c) 2011-2021 Yermalayeu Ihar.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/
#include "Simd/SimdDefs.h"
#include "Simd/SimdMemory.h"
#include "Simd/SimdStore.h"
#include "Simd/SimdTransform.h"

namespace Simd
{
#ifdef SIMD_AVX512BW_ENABLE    
    namespace Avx512bw
    {
        //-----------------------------------------------------------------------------------------

        template<size_t N> void TransformImageRotate180(const uint8_t* src, size_t srcStride, size_t width, size_t height, uint8_t* dst, size_t dstStride)
        {
            dst += (height - 1) * dstStride + (width - Avx2::A) * N;
            size_t widthA = AlignLo(width, Avx2::A);
            size_t widthDA = AlignLo(width, Avx2::DA);
            for (size_t row = 0; row < height; ++row)
            {
                size_t col = 0;
                for (; col < widthDA; col += Avx2::DA)
                    Avx2::TransformImageMirror64<N>(src + col * N, dst - col * N);
                for (; col < widthA; col += Avx2::A)
                    Avx2::TransformImageMirror32<N>(src + col * N, dst - col * N);
                if (col < width)
                    Avx2::TransformImageMirror32<N>(src + (width - Avx2::A) * N, dst - (width - Avx2::A) * N);
                src += srcStride;
                dst -= dstStride;
            }
        }

        template<> void TransformImageRotate180<1>(const uint8_t* src, size_t srcStride, size_t width, size_t height, uint8_t* dst, size_t dstStride)
        {
            dst += (height - 1) * dstStride + (width - 64) * 1;
            size_t width64= AlignLo(width, 64);
            __mmask64 tail = TailMask64(width - width64), nose = NoseMask64(width - width64);
            size_t size = width * 1, size64 = width64 * 1, size256 = AlignLo(size, 256);
            for (size_t row = 0; row < height; ++row)
            {
                size_t offs = 0;
                for (; offs < size256; offs += 256)
                    Avx512bw::TransformImageMirror1x256(src + offs, dst - offs);
                for (; offs < size64; offs += 64)
                    Avx512bw::TransformImageMirror1x64(src + offs, dst - offs);
                if (offs < size)
                    Avx512bw::TransformImageMirror1x64(src + offs, dst - offs, tail, nose);
                src += srcStride;
                dst -= dstStride;
            }
        }

        template<> void TransformImageRotate180<2>(const uint8_t* src, size_t srcStride, size_t width, size_t height, uint8_t* dst, size_t dstStride)
        {
            dst += (height - 1) * dstStride + (width - 32) * 2;
            size_t width32 = AlignLo(width, 32);
            __mmask32 tail = TailMask32(width - width32), nose = NoseMask32(width - width32);
            size_t size = width * 2, size64 = width32 * 2, size256 = AlignLo(size, 256);
            for (size_t row = 0; row < height; ++row)
            {
                size_t offs = 0;
                for (; offs < size256; offs += 256)
                    Avx512bw::TransformImageMirror2x128(src + offs, dst - offs);
                for (; offs < size64; offs += 64)
                    Avx512bw::TransformImageMirror2x32(src + offs, dst - offs);
                if (offs < size)
                    Avx512bw::TransformImageMirror2x32(src + offs, dst - offs, tail, nose);
                src += srcStride;
                dst -= dstStride;
            }
        }

        template<> void TransformImageRotate180<4>(const uint8_t* src, size_t srcStride, size_t width, size_t height, uint8_t* dst, size_t dstStride)
        {
            dst += (height - 1) * dstStride + (width - 16) * 4;
            size_t width16 = AlignLo(width, 16);
            __mmask16 tail = TailMask16(width - width16), nose = NoseMask16(width - width16);
            size_t size = width * 4, size64 = width16 * 4, size256 = AlignLo(size, 256);
            for (size_t row = 0; row < height; ++row)
            {
                size_t offs = 0;
                for (; offs < size256; offs += 256)
                    Avx512bw::TransformImageMirror4x64(src + offs, dst - offs);
                for (; offs < size64; offs += 64)
                    Avx512bw::TransformImageMirror4x16(src + offs, dst - offs);
                if(offs < size)
                    Avx512bw::TransformImageMirror4x16(src + offs, dst - offs, tail, nose);
                src += srcStride;
                dst -= dstStride;
            }
        }

        //-----------------------------------------------------------------------------------------

        template<size_t N> void TransformImageTransposeRotate0(const uint8_t* src, size_t srcStride, size_t width, size_t height, uint8_t* dst, size_t dstStride);

        template<> void TransformImageTransposeRotate0<1>(const uint8_t* src, size_t srcStride, size_t width, size_t height, uint8_t* dst, size_t dstStride)
        {
            size_t width16 = AlignLo(width, 16);
            size_t height16 = AlignLo(height, 16);
            size_t height8 = AlignLo(height, 8);
            size_t row = 0;
            for (; row < height16; row += 16)
            {
                size_t col = 0;
                for (; col < width16; col += 16)
                    Avx2::TransformImageTranspose_1x16x16(src + col * 1, srcStride, dst + col * dstStride, dstStride);
                for (; col < width; ++col)
                    for (size_t i = 0; i < 16; ++i)
                        Base::CopyPixel<1>(src + col * 1 + i * srcStride, dst + col * dstStride + i * 1);
                src += 16 * srcStride;
                dst += 16;
            }
            for (; row < height8; row += 8)
            {
                size_t col = 0;
                for (; col < width16; col += 16)
                    Sse41::TransformImageTranspose_1x8x16(src + col * 1, srcStride, dst + col * dstStride, dstStride);
                for (; col < width; ++col)
                    for (size_t i = 0; i < 8; ++i)
                        Base::CopyPixel<1>(src + col * 1 + i * srcStride, dst + col * dstStride + i * 1);
                src += 8 * srcStride;
                dst += 8;
            }
            for (; row < height; ++row)
            {
                for (size_t col = 0; col < width; ++col)
                    Base::CopyPixel<1>(src + col * 1, dst + col * dstStride);
                src += srcStride;
                dst += 1;
            }
        }

        template<> void TransformImageTransposeRotate0<2>(const uint8_t* src, size_t srcStride, size_t width, size_t height, uint8_t* dst, size_t dstStride)
        {
            size_t width8 = AlignLo(width, 8);
            size_t width16 = AlignLo(width, 16);
            size_t height8 = AlignLo(height, 8);
            size_t height16 = AlignLo(height, 16);
            size_t row = 0;
            for (; row < height16; row += 16)
            {
                size_t col = 0;
                for (; col < width8; col += 8)
                    Avx2::TransformImageTranspose_2x16x8(src + col * 2, srcStride, dst + col * dstStride, dstStride);
                for (; col < width; ++col)
                    for (size_t i = 0; i < 16; ++i)
                        Base::CopyPixel<2>(src + col * 2 + i * srcStride, dst + col * dstStride + i * 2);
                src += 16 * srcStride;
                dst += 32;
            }
            for (; row < height8; row += 8)
            {
                size_t col = 0;
                for (; col < width16; col += 16)
                    Avx2::TransformImageTranspose_2x8x16(src + col * 2, srcStride, dst + col * dstStride, dstStride);
                for (; col < width8; col += 8)
                    Sse41::TransformImageTranspose_2x8x8(src + col * 2, srcStride, dst + col * dstStride, dstStride);
                for (; col < width; ++col)
                    for (size_t i = 0; i < 8; ++i)
                        Base::CopyPixel<2>(src + col * 2 + i * srcStride, dst + col * dstStride + i * 2);
                src += 8 * srcStride;
                dst += 16;
            }
            for (; row < height; ++row)
            {
                for (size_t col = 0; col < width; ++col)
                    Base::CopyPixel<2>(src + col * 2, dst + col * dstStride);
                src += srcStride;
                dst += 2;
            }
        }

        template<> void TransformImageTransposeRotate0<3>(const uint8_t* src, size_t srcStride, size_t width, size_t height, uint8_t* dst, size_t dstStride)
        {
            size_t width4 = AlignLo(width - 5, 4);
            size_t width8 = AlignLo(width - 9, 8);
            size_t height4 = AlignLo(height - 5, 4);
            size_t height8 = AlignLo(height - 9, 8);
            size_t row = 0;
            for (; row < height8; row += 8)
            {
                size_t col = 0;
                for (; col < width4; col += 4)
                    Avx2::TransformImageTranspose_3x8x4(src + col * 3, srcStride, dst + col * dstStride, dstStride);
                for (; col < width; ++col)
                    for (size_t i = 0; i < 8; ++i)
                        Base::CopyPixel<3>(src + col * 3 + i * srcStride, dst + col * dstStride + i * 3);
                src += 8 * srcStride;
                dst += 24;
            }
            for (; row < height4; row += 4)
            {
                size_t col = 0;
                for (; col < width8; col += 8)
                    Avx2::TransformImageTranspose_3x4x8(src + col * 3, srcStride, dst + col * dstStride, dstStride);
                for (; col < width4; col += 4)
                    Sse41::TransformImageTranspose_3x4x4(src + col * 3, srcStride, dst + col * dstStride, dstStride);
                for (; col < width; ++col)
                    for (size_t i = 0; i < 4; ++i)
                        Base::CopyPixel<3>(src + col * 3 + i * srcStride, dst + col * dstStride + i * 3);
                src += 4 * srcStride;
                dst += 12;
            }
            for (; row < height; ++row)
            {
                for (size_t col = 0; col < width; ++col)
                    Base::CopyPixel<3>(src + col * 3, dst + col * dstStride);
                src += srcStride;
                dst += 3;
            }
        }

        template<> void TransformImageTransposeRotate0<4>(const uint8_t* src, size_t srcStride, size_t width, size_t height, uint8_t* dst, size_t dstStride)
        {
            size_t width4 = AlignLo(width, 4);
            size_t width8 = AlignLo(width, 8);
            size_t width16 = AlignLo(width, 16);
            size_t height4 = AlignLo(height, 4);
            size_t height8 = AlignLo(height, 8);
            size_t height16 = AlignLo(height, 16);

            size_t row = 0;
            for (; row < height16; row += 16)
            {
                size_t col = 0;
                for (; col < width16; col += 16)
                    Avx512bw::TransformImageTranspose_4x16x16(src + col * 4, srcStride, dst + col * dstStride, dstStride);
                for (; col < width8; col += 8)
                    Avx512bw::TransformImageTranspose_4x16x8(src + col * 4, srcStride, dst + col * dstStride, dstStride);
                for (; col < width; ++col)
                    for (size_t i = 0; i < 16; ++i)
                        Base::CopyPixel<4>(src + col * 4 + i * srcStride, dst + col * dstStride + i * 4);
                src += 16 * srcStride;
                dst += 64;
            }
            for (; row < height8; row += 8)
            {
                size_t col = 0;
                for (; col < width8; col += 8)
                    Avx2::TransformImageTranspose_4x8x8(src + col * 4, srcStride, dst + col * dstStride, dstStride);
                for (; col < width4; col += 4)
                    Avx2::TransformImageTranspose_4x8x4(src + col * 4, srcStride, dst + col * dstStride, dstStride);
                for (; col < width; ++col)
                    for (size_t i = 0; i < 8; ++i)
                        Base::CopyPixel<4>(src + col * 4 + i * srcStride, dst + col * dstStride + i * 4);
                src += 8 * srcStride;
                dst += 32;
            }
            for (; row < height4; row += 4)
            {
                size_t col = 0;
                for (; col < width8; col += 8)
                    Avx2::TransformImageTranspose_4x4x8(src + col * 4, srcStride, dst + col * dstStride, dstStride);
                for (; col < width4; col += 4)
                    Sse41::TransformImageTranspose_4x4x4(src + col * 4, srcStride, dst + col * dstStride, dstStride);
                for (; col < width; ++col)
                    for (size_t i = 0; i < 4; ++i)
                        Base::CopyPixel<4>(src + col * 4 + i * srcStride, dst + col * dstStride + i * 4);
                src += 4 * srcStride;
                dst += 16;
            }
            for (; row < height; ++row)
            {
                for (size_t col = 0; col < width; ++col)
                    Base::CopyPixel<4>(src + col * 4, dst + col * dstStride);
                src += srcStride;
                dst += 4;
            }
        }

        //-----------------------------------------------------------------------------------------

        template<size_t N> void TransformImageTransposeRotate90(const uint8_t* src, size_t srcStride, size_t width, size_t height, uint8_t* dst, size_t dstStride)
        {
            dst += (width - Avx2::A) * N;
            size_t widthA = AlignLo(width, Avx2::A);
            size_t widthDA = AlignLo(width, Avx2::DA);
            for (size_t row = 0; row < height; ++row)
            {
                size_t col = 0;
                for (; col < widthDA; col += Avx2::DA)
                    Avx2::TransformImageMirror64<N>(src + col * N, dst - col * N);
                for (; col < widthA; col += Avx2::A)
                    Avx2::TransformImageMirror32<N>(src + col * N, dst - col * N);
                if (col < width)
                    Avx2::TransformImageMirror32<N>(src + (width - Avx2::A) * N, dst - (width - Avx2::A) * N);
                src += srcStride;
                dst += dstStride;
            }
        }

        template<> void TransformImageTransposeRotate90<1>(const uint8_t* src, size_t srcStride, size_t width, size_t height, uint8_t* dst, size_t dstStride)
        {
            dst += (width - 64) * 1;
            size_t width64 = AlignLo(width, 64);
            __mmask64 tail = TailMask64(width - width64), nose = NoseMask64(width - width64);
            size_t size = width * 1, size64 = width64 * 1, size256 = AlignLo(size, 256);
            for (size_t row = 0; row < height; ++row)
            {
                size_t offs = 0;
                for (; offs < size256; offs += 256)
                    Avx512bw::TransformImageMirror1x256(src + offs, dst - offs);
                for (; offs < size64; offs += 64)
                    Avx512bw::TransformImageMirror1x64(src + offs, dst - offs);
                if (offs < size)
                    Avx512bw::TransformImageMirror1x64(src + offs, dst - offs, tail, nose);
                src += srcStride;
                dst += dstStride;
            }
        }

        template<> void TransformImageTransposeRotate90<2>(const uint8_t* src, size_t srcStride, size_t width, size_t height, uint8_t* dst, size_t dstStride)
        {
            dst += (width - 32) * 2;
            size_t width32 = AlignLo(width, 32);
            __mmask32 tail = TailMask32(width - width32), nose = NoseMask32(width - width32);
            size_t size = width * 2, size64 = width32 * 2, size256 = AlignLo(size, 256);
            for (size_t row = 0; row < height; ++row)
            {
                size_t offs = 0;
                for (; offs < size256; offs += 256)
                    Avx512bw::TransformImageMirror2x128(src + offs, dst - offs);
                for (; offs < size64; offs += 64)
                    Avx512bw::TransformImageMirror2x32(src + offs, dst - offs);
                if (offs < size)
                    Avx512bw::TransformImageMirror2x32(src + offs, dst - offs, tail, nose);
                src += srcStride;
                dst += dstStride;
            }
        }

        template<> void TransformImageTransposeRotate90<4>(const uint8_t* src, size_t srcStride, size_t width, size_t height, uint8_t* dst, size_t dstStride)
        {
            dst += (width - 16) * 4;
            size_t width16 = AlignLo(width, 16);
            __mmask16 tail = TailMask16(width - width16), nose = NoseMask16(width - width16);
            size_t size = width * 4, size64 = width16 * 4, size256 = AlignLo(size, 256);;
            for (size_t row = 0; row < height; ++row)
            {
                size_t offs = 0;
                for (; offs < size256; offs += 256)
                    Avx512bw::TransformImageMirror4x64(src + offs, dst - offs);
                for (; offs < size64; offs += 64)
                    Avx512bw::TransformImageMirror4x16(src + offs, dst - offs);
                if (offs < size)
                    Avx512bw::TransformImageMirror4x16(src + offs, dst - offs, tail, nose);
                src += srcStride;
                dst += dstStride;
            }
        }

        //-----------------------------------------------------------------------------------------

        template<size_t N> void Init(ImageTransforms::TransformPtr transforms[8])
        {
            //transforms[SimdTransformRotate90] = TransformImageRotate90<N>;
            transforms[SimdTransformRotate180] = TransformImageRotate180<N>;
            //transforms[SimdTransformRotate270] = TransformImageRotate270<N>;
            transforms[SimdTransformTransposeRotate0] = TransformImageTransposeRotate0<N>;
            transforms[SimdTransformTransposeRotate90] = TransformImageTransposeRotate90<N>;
            //transforms[SimdTransformTransposeRotate180] = TransformImageTransposeRotate180<N>;
        }

        ImageTransforms::ImageTransforms()
            : Avx2::ImageTransforms::ImageTransforms()
        {
            Init<1>(transforms[0]);
            Init<2>(transforms[1]);
            //Init<3>(transforms[2]);
            Init<4>(transforms[3]);
        }

        //-----------------------------------------------------------------------------------------

        void TransformImage(const uint8_t * src, size_t srcStride, size_t width, size_t height, size_t pixelSize, SimdTransformType transform, uint8_t * dst, size_t dstStride)
        {
            static ImageTransforms transforms = ImageTransforms();

            transforms.TransformImage(src, srcStride, width, height, pixelSize, transform, dst, dstStride);
        }
    }
#endif
}
