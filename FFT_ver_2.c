#include <stdio.h>
#include <stdlib.h> 
#include <math.h>

int main()
{
	int i;
	const int N = 16;
	double y_re[N], y_im[N], x_re[N], x_im[N];
	for(i=0;i<N;++i)
	{
		x_re[i] = i;
		x_im[i] = 0.0;
	}
	bit_reverse(x_re, x_im, N);	
	butterfly(x_re, x_im, N);
	
	for(i=0;i<N;++i)
	{
		printf("%f + %f i\n", x_re[i], x_im[i]);
	}
	return;
	 
}

int bit_reverse(double *x_re, double *x_im, int N)
{
    int m,p,q,k;
    double t;
    
    m = N/2;                        // Bit-Reverse �C���n�i�쪺�Ʀr 
    q = m;							// p = 1, q = m (�Ĥ@�ӭn�洫��)
    // p = 0 �M p = N-1 ��ۤv�洫�ҥH���L 
    for(p=1;p<N-1;++p)
    {
        printf("%d <-> %d\n", p,q);
        if(p < q)
        {
            t = x_re[p];
            x_re[p] = x_re[q];
			x_re[q] = t;
            t = x_im[p];
            x_im[p] = x_im[q];
			x_im[q] = t;			 
        }
        k = m;						// k, �Ψ��ˬd�� log_2 k + 1 ��O���O1
									// take N = 16 for example, k = m = N/2 = 8 
        							// (1110) = 14 >= (8=k), log_2 k + 1 = 4
									// (0110) = 6  >= (4=k), log_2 k + 1 = 3
									// (0010) = 2  >= (2=k)
									// (0000) = 0  >= (1=k) X break --> (0001)
									// (0110) = 6  >= (8=k) X break --> (1110) 
        while(q >= k & k > 0)		// q >=k �� (log_2 k + 1)��O1, !!! �Pık>0�O���ݭn������A�T�w��R�� !!! 
        {
            q = q-k;				// 1->0
            k = k/2;				// �ˬd�U�@�� 
        }
        q = q+k;
    }
    return 0;
}
int butterfly(double *x_re, double *x_im, int N)
{
	int k, p, q, m;
	// k���p���k��butterflu�k��X�Ȥ�index, k from 0 to m
	// p�Obutterfly��1��J�Ȥ�index
	// q�Obutterfly��2��J�Ȥ�index
	// m�O�C�խӼ�/2�A�]�Op, q�����Z 
	double w_re, w_im, w_N_re, w_N_im, t; 
	m = 1;
	while(m<N)
	{
		w_re = 1.0;		// Re(W_{2^m}^0)
		w_im = 0.0;		// Im(W_{2^m}^0)
		w_N_re =  cos(M_PI/m);	// Re(W_{2^m}^1)
		w_N_im = -sin(M_PI/m);	// Im(W_{2^m}^1), note that here is a minus sign for W_N = e^{-2PI/N}
		for(k=0;k<m;++k) 
		{
			for(p=k;p<N;p+=2*m)
			{	
				// find index of q
				q = p + m;
				// multiply W_{2^m}^k on x[q]
				// or say, multiply (w_re + w_im * i) on x[q]
				t = x_re[q]; 
				x_re[q] = w_re*x_re[q] - w_im*x_im[q];
				x_im[q] = w_re*x_im[q] + w_im*t; 
				
				// apply butterfly structure to
				// calculate x_p and x_q (counterpart)
				// here we calculate by input multiplied with FFT_2_Matrix(multiplier on butterfly)
				t = x_re[p];
				x_re[p] = x_re[p] + x_re[q];
				x_re[q] = t       - x_re[q]; 
				t = x_im[p];
				x_im[p] = x_im[p] + x_im[q];
				x_im[q] = t       - x_im[q]; 
				
				
			}
			// Calculate each multiplier of counterpart
			// say, W_{2^m}^k
			// it's recurrsive, like W_{2^m}^k = W_{2^m}^{k-1} * W_{2^m}^1
			t    = w_re; 
			w_re = w_N_re*w_re - w_N_im*w_im;
			w_im = w_N_re*w_im + w_N_im*t;
		}
		// to next step of FT
		m = m * 2;
	}
	
	return;
}



int Fast_Fourier_Transform(double *y_re, double *y_im, double *x_re, double *x_im, int N)
{
	if(N==2) 
	{
		// y, y[0] = x[0]+x[1], y[1] = x[0] - x[1]
		y_re[0] = x_re[0] + x_re[1];
		y_im[0] = x_im[0] + x_im[1];
		y_re[1] = x_re[0] - x_re[1]; 
		y_im[1] = x_im[0] - x_im[1];
	} else 
	{
		int k;
		double *y_even_re, *y_even_im, *y_odd_re, *y_odd_im;
		double *x_even_re, *x_even_im, *x_odd_re, *x_odd_im;
		double w_re, w_im, w_N_re, w_N_im, a, b, temp;
		y_even_re = (double *) malloc( N/2 * sizeof(double));
		y_even_im = (double *) malloc( N/2 * sizeof(double));
		x_even_re = (double *) malloc( N/2 * sizeof(double));
		x_even_im = (double *) malloc( N/2 * sizeof(double));
		y_odd_re = (double *) malloc( N/2 * sizeof(double));
		y_odd_im = (double *) malloc( N/2 * sizeof(double));
		x_odd_re = (double *) malloc( N/2 * sizeof(double));
		x_odd_im = (double *) malloc( N/2 * sizeof(double));
		for(k=0;k<N/2;++k)
		{
			x_even_re[k] = x_re[2*k];
			x_even_im[k] = x_im[2*k];
			x_odd_re[k]  = x_re[2*k+1];
			x_odd_im[k]  = x_im[2*k+1];
		}
		Fast_Fourier_Transform(y_even_re, y_even_im, x_even_re, x_even_im, N/2);
		Fast_Fourier_Transform(y_odd_re, y_odd_im, x_odd_re, x_odd_im, N/2);
		// y_k = even_k + w_N^k odd_k = even_k + (a + bi)
		w_N_re =  cos(2.0*M_PI/N);
		w_N_im = -sin(2.0*M_PI/N);
		w_re   = 1.0;
		w_im   = 0.0; 
		for(k=0;k<N/2;++k)
		{
			a = w_re*y_odd_re[k] - w_im*y_odd_im[k];
			b = w_re*y_odd_im[k] + w_im*y_odd_re[k];
			y_re[k]     = y_even_re[k] + a;
			y_im[k]     = y_even_im[k] + b;
			y_re[N/2+k] = y_even_re[k] - a;
			y_im[N/2+k] = y_even_im[k] - b;
			temp = w_re;
			w_re = w_re*w_N_re - w_im*w_N_im;
			w_im = temp*w_N_im + w_im*w_N_re;
		}
		free(y_even_re);
		free(x_even_re);
		free(y_even_im);
		free(x_even_im);
		free(y_odd_re);
		free(y_odd_im);
		free(x_odd_re);
		free(x_odd_im);
	}
}
