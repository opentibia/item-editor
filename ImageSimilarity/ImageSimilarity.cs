using System;
using System.Collections.Generic;
using System.Text;
using System.Drawing;
using System.Drawing.Imaging;

namespace ImageSimilarity
{
	public class ImageSimilarity
	{
		public static void CompareImage(Bitmap bitmap1, Bitmap bitmap2, int blockSize, out double similarity)
		{
			similarity = 0.0;

			Bitmap ff2dbitmap1 = Fourier.fft2d(bitmap1, false);
			Bitmap ff2dbitmap2 = Fourier.fft2d(bitmap2, false);

			double[,] keySignature = ImageUtils.CalculateEuclideanDistance(ff2dbitmap1, blockSize);
			double[,] compareSignature = ImageUtils.CalculateEuclideanDistance(ff2dbitmap2, blockSize);

			similarity = ImageUtils.CompareSignature(keySignature, compareSignature);
		}

		public static void CompareImageRGB(Bitmap bitmap1, Bitmap bitmap2, int blockSize, out double similarity)
		{
			similarity = 0.0;

			Bitmap ff2dbitmap1 = Fourier.fft2dRGB(bitmap1, true);
			Bitmap ff2dbitmap2 = Fourier.fft2dRGB(bitmap2, true);

			double[,] keySignature = ImageUtils.CalculateEuclideanDistance(ff2dbitmap1, blockSize);
			double[,] compareSignature = ImageUtils.CalculateEuclideanDistance(ff2dbitmap2, blockSize);

			similarity = ImageUtils.CompareSignature(keySignature, compareSignature);
		}
	}
}
