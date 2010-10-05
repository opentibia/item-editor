using System;
using System.Collections.Generic;
using System.Text;
using System.IO;
using System.Drawing;
using System.Drawing.Imaging;

namespace otitemeditor
{
	public class Utils
	{
		public static bool ByteArrayCompare(byte[] a1, byte[] a2)
		{
			if (a1.Length != a2.Length)
				return false;

			for (int i = 0; i < a1.Length; i++)
				if (a1[i] != a2[i])
					return false;

			return true;
		}

		public static Bitmap getBitmap(byte[] rgbData, int Width, int Height, int bitPerPixel)
		{
			Bitmap output = new Bitmap(Width, Height, PixelFormat.Format24bppRgb);
			BitmapData outputData = output.LockBits(new Rectangle(0, 0, output.Width, output.Height), ImageLockMode.ReadWrite, output.PixelFormat);

			//reverse rgb
			for (int y = 0; y < Height; ++y)
			{
				for (int x = 0; x < Width; ++x)
				{
					if (bitPerPixel == 3)
					{
						byte r = rgbData[Width * bitPerPixel * y + x * bitPerPixel + 0];
						byte g = rgbData[Width * bitPerPixel * y + x * bitPerPixel + 1];
						byte b = rgbData[Width * bitPerPixel * y + x * bitPerPixel + 2];

						rgbData[Width * bitPerPixel * y + x * bitPerPixel + 0] = b;
						rgbData[Width * bitPerPixel * y + x * bitPerPixel + 1] = g;
						rgbData[Width * bitPerPixel * y + x * bitPerPixel + 2] = r;
					}
				}
			}

			System.Runtime.InteropServices.Marshal.Copy(rgbData, 0, outputData.Scan0, rgbData.Length);
			output.UnlockBits(outputData);
			return output;
		}

		/// <summary>
		/// Greyscale (currently Luminance)
		/// </summary>
		/// <param name="input"></param>
		/// <returns></returns>+
		public static byte[] greyScale(Bitmap input)
		{
			BitmapData inputData = input.LockBits(new Rectangle(0, 0, input.Width, input.Height), ImageLockMode.ReadWrite, input.PixelFormat);
			IntPtr inputIntPtr = inputData.Scan0;
			int width = input.Width;
			int height = input.Height;
			byte[] data = new byte[input.Width * input.Height * 3];
			System.Runtime.InteropServices.Marshal.Copy(inputIntPtr, data, 0, data.Length);
			input.UnlockBits(inputData);
			byte[] greyscale = new byte[width * height];
			int counter = 0;
			for (int i = 0; i < data.Length; i += 3)
			{
				greyscale[counter++] = (byte)(((66 * data[i + 2] + 129 * data[i] + 25 * data[i + 1] + 128) >> 8) + 16);
			}
			return greyscale;
		}


		public static double compareFF2Signature(double[] origSignature, double[] compareSignature)
		{
			double dTemp = 0.0;
			for (int i = 0; i < compareSignature.Length; i++)
			{
				dTemp += (origSignature[i] - compareSignature[i]) * (origSignature[i] - compareSignature[i]);
			}

			dTemp = Math.Sqrt(dTemp);
			return dTemp;
		}

		public static double compareImage(Bitmap orig, Bitmap input)
		{
			double[] keySignature = getEuclideanDistance(orig);
			double[] compareSignature = getEuclideanDistance(input);

			return compareFF2Signature(keySignature, compareSignature);
		}

		public static double[] getEuclideanDistance(Bitmap input)
		{
			BitmapData bmpData = input.LockBits(
							 new Rectangle(0, 0, input.Width, input.Height),
							 ImageLockMode.WriteOnly, input.PixelFormat);

			// Declare an array to hold the bytes of the bitmap.
			int bytes = bmpData.Stride * input.Height;
			byte[] rgbValues = new byte[bytes];

			// Copy the RGB values into the array.
			System.Runtime.InteropServices.Marshal.Copy(bmpData.Scan0, rgbValues, 0, bytes);
			input.UnlockBits(bmpData);

			int blockSize = 4;
			int blockNum = 0;
			double[] lSignature = new double[(input.Width / blockSize) * (input.Height / blockSize)];
			double dTemp = 0.0;

			MemoryStream memStream = new MemoryStream(rgbValues, 0, rgbValues.Length);
			BinaryReader reader = new BinaryReader(memStream);

			for (int y = 0; y < input.Height; y += blockSize)
			{
				for (int x = 0; x < input.Width; x += blockSize)
				{
					lSignature[blockNum] = 0.0;

					for (int blocky = 0; blocky < blockSize; ++blocky)
					{
						for (int blockx = 0; blockx < blockSize; ++blockx)
						{
							dTemp += reader.ReadUInt16();
						}
					}

					lSignature[blockNum] = Math.Sqrt(dTemp);
					++blockNum;
				}
			}

			// Normalize
			for (int i = 0; i < lSignature.Length; i++)
			{
				dTemp += lSignature[i];
			}

			for (int i = 0; i < lSignature.Length; i++)
			{
				lSignature[i] /= dTemp;
			}

			return lSignature;
		}
	}
}
