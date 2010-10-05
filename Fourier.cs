using System;
using System.Drawing;

namespace otitemeditor
{
	public class Fourier
	{
		private Fourier()
		{

		}

		public static Complex[] fft(Complex[] x)
		{
			int length = x.Length;
			// 
			if (length == 1) return new Complex[] { x[0] };

			// Cooley-Tukey FFT
			if (length % 2 != 0) throw new Exception("N is not a power of 2");

			// even fft
			Complex[] even = new Complex[length / 2];
			for (int k = 0; k < length / 2; k++)
			{
				even[k] = x[2 * k];
			}
			Complex[] q = fft(even);

			// odd fft;
			Complex[] odd = even;
			for (int k = 0; k < length / 2; k++)
			{
				odd[k] = x[2 * k + 1];
			}
			Complex[] r = fft(odd);

			// combine
			Complex[] y = new Complex[length];
			for (int k = 0; k < length / 2; k++)
			{
				double value = -2 * k * Math.PI / length;
				Complex wk = new Complex(Math.Cos(value), Math.Sin(value));
				y[k] = q[k] + (wk * (r[k]));
				y[k + length / 2] = q[k] - (wk * (r[k]));
			}
			return y;
		}

		public static Complex[] ifft(Complex[] x)
		{
			int length = x.Length;
			Complex[] y = new Complex[length];

			// Cooley-Tukey FFT
			if (length % 2 != 0) throw new Exception("N is not a power of 2");

			//conjugate
			for (int i = 0; i < length; i++)
			{
				y[i] = new Complex(x[i].Re, -x[i].Im);
			}

			// compute forward FFT
			y = fft(y);

			// take conjugate again
			for (int i = 0; i < length; i++)
			{
				y[i] = new Complex(y[i].Re, -y[i].Im);
			}

			// divide by N
			for (int i = 0; i < length; i++)
			{
				y[i] = y[i] / length;
			}

			return y;
		}

		public static Complex[,] fft2d(Complex[,] input)
		{
			Complex[,] output = (Complex[,])input.Clone();
			// Rows first:
			Complex[] x = new Complex[output.GetLength(1)];
			for (int h = 0; h < output.GetLength(0); h++)
			{
				for (int i = 0; i < output.GetLength(1); i++)
				{
					x[i] = output[h, i];
				}
				x = fft(x);
				for (int i = 0; i < output.GetLength(1); i++)
				{
					output[h, i] = x[i];
				}
			}
			//Columns last
			Complex[] y = new Complex[output.GetLength(0)];
			for (int h = 0; h < output.GetLength(1); h++)
			{
				for (int i = 0; i < output.GetLength(0); i++)
				{
					y[i] = output[i, h];
				}
				y = fft(y);
				for (int i = 0; i < output.GetLength(0); i++)
				{
					output[i, h] = y[i];
				}
			}
			return output;
		}

		public static Complex[,] ifft2d(Complex[,] input)
		{
			Complex[,] output = (Complex[,])input.Clone();
			// Rows first:
			Complex[] x = new Complex[output.GetLength(1)];
			for (int h = 0; h < output.GetLength(0); h++)
			{
				for (int i = 0; i < output.GetLength(1); i++)
				{
					x[i] = output[h, i];
				}
				x = ifft(x);
				for (int i = 0; i < output.GetLength(1); i++)
				{
					output[h, i] = x[i];
				}
			}
			//Columns last
			Complex[] y = new Complex[output.GetLength(0)];
			for (int h = 0; h < output.GetLength(1); h++)
			{
				for (int i = 0; i < output.GetLength(0); i++)
				{
					y[i] = output[i, h];
				}
				y = ifft(y);
				for (int i = 0; i < output.GetLength(0); i++)
				{
					output[i, h] = y[i];
				}
			}
			return output;
		}

		public static Bitmap fft2d(Bitmap input, bool reorder)
		{
			int width = input.Width;
			int height = input.Height;
			byte[] data = Utils.greyScale(input);
			Complex[,] cmplx = new Complex[height, width];
			double scale = 1.0 / (double)Math.Sqrt(width * height);
			for (int i = 0; i < data.Length; i++)
			{
				cmplx[i / width, i % width] = new Complex(data[i] / 256.0);
			}
			cmplx = fft2d(cmplx);
			for (int i = 0; i < data.Length; i++)
			{
				data[i] = (byte)Math.Min(255, (int)(cmplx[i / width, i % width].GetModulus() * 256.0) * scale);
			}
			// Swapping data -> lowest frequencies at center
			if (reorder)
			{
				for (int i = 0; i < height / 2; i++)
				{
					for (int k = 0; k < width / 2; k++)
					{
						byte val1 = data[i * width + k];
						byte val2 = data[i * width + k + width / 2];
						data[i * width + k] = data[(i + width / 2) * width + k + width / 2];
						data[(i + width / 2) * width + k + width / 2] = val1;
						data[i * width + k + width / 2] = data[(i + width / 2) * width + k];
						data[(i + width / 2) * width + k] = val2;
					}
				}
			}
			Bitmap output = Utils.getBitmap(data, width, height, 1);
			return output;
		}

		public static Bitmap ifft2d(Bitmap input)
		{
			int width = (int)Math.Pow(2, Math.Ceiling(Math.Log(input.Width, 2)));
			int height = (int)Math.Pow(2, Math.Ceiling(Math.Log(input.Width, 2)));
			byte[] data = Utils.greyScale(input);
			if (width != input.Width || height != input.Height)
			{
				// Pad data
				byte[] newdata = new byte[width * height];
				int xoff = width - input.Width;
				int yoff = height - input.Height;
				for (int i = 0; i < input.Height; i++)
				{
					for (int k = 0; k < input.Width; k++)
					{
						newdata[(i + yoff) * width + k + xoff] = data[i * input.Width + k];
					}
				}
				data = newdata;
			}
			Complex[,] cmplx = new Complex[height, width];
			double scale = 1.0 / (double)Math.Sqrt(width * height);
			scale = 1;
			for (int i = 0; i < data.Length; i++)
			{
				cmplx[i / width, i % width] = new Complex(data[i] / 256.0);
			}
			cmplx = ifft2d(cmplx);
			for (int i = 0; i < height; i++)
			{
				for (int k = 0; k < width; k++)
				{
					if (((k + i) & 0x1) != 0)
					{
						cmplx[i, k] *= -1;
					}
				}
			}
			for (int i = 0; i < data.Length; i++)
			{
				data[i] = (byte)Math.Min(255, (int)(cmplx[i / width, i % width].GetModulus() * 256.0 * scale));
			}
			Bitmap output = Utils.getBitmap(data, width, height, 1);
			return output;
		}
	}
}
