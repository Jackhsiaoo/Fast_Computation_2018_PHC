#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>
#define DEBUG 0

/// Function declaration
void FFT(double *x_re, double *x_im, int N);
void bit_reverse(double *x_re, double *x_im, int N);
void butterfly5(double *x_re, double *x_im, int N_st, int N_end, int N);
void butterfly3(double *x_re, double *x_im, int N_st, int N_end, int N);
void butterfly2(double *x_re, double *x_im, int N_st, int N_end, int N);


void FFT(double *x_re, double *x_im, int N){
    int d_2 = 0;
	int d_3 = 0;
	int d_5 = 0;
	int N_st, N_end;

	// calculate degree
    int n_temp = N;		// for calculation of each degree
    while(n_temp%2==0){
    	d_2++;
    	n_temp /= 2;
	}
    while(n_temp%3==0){
    	d_3++;
    	n_temp /= 3;
	}
    while(n_temp%5==0){
    	d_5++;
    	n_temp /= 5;
	}

    // bit-reverse
    bit_reverse(x_re, x_im, N);

	// butterfly 5
    N_st = 1;
    N_end = pow(5,d_5) + 0.5D;
    butterfly5(x_re, x_im, N_st, N_end, N);
    // butterfly 3
    N_st = N_end;
    N_end = pow(3,d_3)*pow(5,d_5) + 0.5D;
    butterfly3(x_re, x_im, N_st, N_end, N);
    // butterfly 2
    N_st = N_end;
    N_end = N;
    butterfly2(x_re, x_im, N_st, N_end, N);
}



/// bit reverse for setting all input in place
void bit_reverse(double *x_re, double *x_im, int N){
	int base, base_max, N_base;
	int id_b;   // index for base
    int m;    // highest bit
    int p, q;       // p & q is the index that exchanges for each other
    int k;          // k is use to check digital (log_2 k + 1) if is 1
    int record;
    int i;
    double temp;


    // set base
    i=0;
	if (N%2 == 0){
		base = 2;
		i++;
	}
	if (N%3 == 0){
		base = 3;
		i++;
	}
	if (N%5 == 0){
		base = 5;
		i++;
	}
	N_base = --i;
    base_max = base;

	// set temporary array
	double *y_re, *y_im;
	if (N_base != 0){
	    y_re = (double*) malloc(N*sizeof(double));
	    y_im = (double*) malloc(N*sizeof(double));
	    for (i=0; i<N; i++){
	        y_re[i] = x_re[i];
	        y_im[i] = x_im[i];
	    }
	}


    // p is index before exchanging
    // from 0 to N-1 regularly
    // q is index after exchanging
    m = N/base;    // added number which is highest bit
    q = m;  // first index of exchanged number
    // skip p = 0 & N-1 because they are exchanged by themselves
    for (p=1; p<N-1; p++){

        #if DEBUG
        printf("%d <-> %d\n", p, q);
        #endif

        // assign x to new place
		if (N_base != 0){
			x_re[p] = y_re[q];
        	x_im[p] = y_im[q];
		}
		else if(p<q){
            temp = x_re[p];
            x_re[p] = x_re[q];
            x_re[q] = temp;
            temp = x_im[p];
            x_im[p] = x_im[q];
            x_im[q] = temp;
        }

        // find next exchanged index q
        // add highest bit into old q
        k = m;
        base = base_max;
        // if it needs to add 1 to right digital
        while(q>=(base-1)*k){
            q = q-(base-1)*k;    // (base-1) -> 0
            if (N_base!=0)
                if ((k%base) != 0){
                    if (base==5){
                    	base = 3;
                    	if ((k%base) != 0)
                        	base = 2;
					}
                    else
                        base = 2;
                }
            k = k/base;    // check next (right) digital
        }
        q = q+k;
    }

    // free memory
    if (N_base != 0){
    	free(y_re);
    	free(y_im);
	}
}



/// butterfly structure to recombine these input
void butterfly5(double *x_re, double *x_im, int N_st, int N_end, int N){
	int base = 5;
    int m;  // m is third of number in each group
            // it's also the difference between z and u
    int k;  // index in RHS of butterfly (output) (from 0 to m)
            // the rest of m/2 is counterpart of the first m/2
    int p;  // the first index in LHS of butterfly (input1)
    int q;  // the second index in LHS of butterfly (input2)
    int s;  // the third index in LHS of butterfly (input3)
    int t;  // the 4th index in LHS of butterfly (input4)
    int r;  // the 5th index in LHS of butterfly (input5)

    double w_k_re, w_k_im, w_N_re, w_N_im;
    double w_2k_re, w_2k_im, w_N_2_re, w_N_2_im;
    double w_3k_re, w_3k_im, w_N_3_re, w_N_3_im;
    double w_4k_re, w_4k_im, w_N_4_re, w_N_4_im;
    double w_but_re_11, w_but_re_12, w_but_re_13, w_but_re_14, \
		   w_but_re_22, w_but_re_23, w_but_re_24,	\
		   w_but_re_33, w_but_re_34, w_but_re_44;
    double w_but_im_11, w_but_im_12, w_but_im_13, w_but_im_14, \
		   w_but_im_22, w_but_im_23, w_but_im_24,	\
		   w_but_im_33, w_but_im_34, w_but_im_44;
    int i, j;
    int N_thre = omp_get_max_threads();

    w_but_re_11 =  cos((2.0D*M_PI/base));
    w_but_re_12 =  cos((2.0D*M_PI/base)*2.0D);
    w_but_re_13 =  cos((2.0D*M_PI/base)*3.0D);
    w_but_re_14 =  cos((2.0D*M_PI/base)*4.0D);
    w_but_re_22 =  cos((2.0D*M_PI/base)*4.0D);
    w_but_re_23 =  cos((2.0D*M_PI/base)*6.0D);
    w_but_re_24 =  cos((2.0D*M_PI/base)*8.0D);
    w_but_re_33 =  cos((2.0D*M_PI/base)*9.0D);
    w_but_re_34 =  cos((2.0D*M_PI/base)*12.0D);
    w_but_re_44 =  cos((2.0D*M_PI/base)*16.0D);
    w_but_im_11 =  -sin((2.0D*M_PI/base));
    w_but_im_12 =  -sin((2.0D*M_PI/base)*2.0D);
    w_but_im_13 =  -sin((2.0D*M_PI/base)*3.0D);
    w_but_im_14 =  -sin((2.0D*M_PI/base)*4.0D);
    w_but_im_22 =  -sin((2.0D*M_PI/base)*4.0D);
    w_but_im_23 =  -sin((2.0D*M_PI/base)*6.0D);
    w_but_im_24 =  -sin((2.0D*M_PI/base)*8.0D);
    w_but_im_33 =  -sin((2.0D*M_PI/base)*9.0D);
    w_but_im_34 =  -sin((2.0D*M_PI/base)*12.0D);
    w_but_im_44 =  -sin((2.0D*M_PI/base)*16.0D);


    // loop for each steps of butterfly (step no.)
    for (m=N_st; m<N_end; m*=base){

        #pragma omp parallel num_threads(N_thre) private(k,p,q,s,t,r)
        {
            int id_t = omp_get_thread_num();
			double temp, temp1_re, temp2_re, temp3_re, temp4_re, temp5_re;   // for temporary storage of number
            double temp1_im, temp2_im, temp3_im, temp4_im, temp5_im;	// 1 for p, 2 for q, 3 for s, 4 for t, 5 for r
            // Calculate multiplier of counterpart
            double w_k_re = cos(2.0D*M_PI/(m*base)*id_t);               // Re(W_{5^m}^{1*k}), multiplier of first counterpart
            double w_k_im = -sin(2.0D*M_PI/(m*base)*id_t);              // Im(W_{5^m}^{1*k})
            double w_2k_re = w_k_re*w_k_re - w_k_im*w_k_im;             // Re(W_{5^m}^{2*k})
            double w_2k_im = 2.0D*w_k_re*w_k_im;                        // Re(W_{5^m}^{2*0})
            double w_3k_re = w_2k_re*w_k_re - w_2k_im*w_k_im;           // Re(W_{5^m}^{3*k})
            double w_3k_im = w_2k_re*w_k_im + w_2k_im*w_k_re;           // Re(W_{5^m}^{3*k})
            double w_4k_re = w_3k_re*w_k_re - w_3k_im*w_k_im;           // Re(W_{5^m}^{4*k})
            double w_4k_im = w_3k_re*w_k_im + w_3k_im*w_k_re;           // Re(W_{5^m}^{4*k})
            double w_N_re =  cos(2.0D*M_PI/(m*base)*N_thre);            // Re(W_{5^m}^{1*N_thre})
            double w_N_im = -sin(2.0D*M_PI/(m*base)*N_thre);            // Im(W_{5^m}^{1*N_thre})
            double w_N_2_re = w_N_re*w_N_re - w_N_im*w_N_im;  		    // Re(W_{5^m}^{2*N_thre})
            double w_N_2_im = 2.0D*w_N_re*w_N_im;  				        // Im(W_{5^m}^{2*N_thre})
            double w_N_3_re = w_N_2_re*w_N_re - w_N_2_im*w_N_im;        // Re(W_{5^m}^{3*N_thre})
            double w_N_3_im = w_N_2_re*w_N_im + w_N_2_im*w_N_re;  	    // Im(W_{5^m}^{3*N_thre})
            double w_N_4_re = w_N_3_re*w_N_re - w_N_3_im*w_N_im;  	    // Re(W_{5^m}^{4*N_thre})
            double w_N_4_im = w_N_3_re*w_N_im + w_N_3_im*w_N_re;  	    // Im(W_{5^m}^{4*N_thre})

            // loop for each output in a group (output no.)
            for (k=id_t; k<m; k+=N_thre){
                // loop for each group (group no.)
                for (p=k; p<N; p+=base*m){
                    // Calculate output of butterfly
                    // find index of counterpart q
                    q = p + m;
                    s = q + m;
                    t = s + m;
                    r = t + m;
                    // apply multiplier of counterpart
                    // say, multiply W_{2^m}^k on x[q]
                    temp = x_re[q];
                    x_re[q] = w_k_re*x_re[q] - w_k_im*x_im[q];
                    x_im[q] = w_k_re*x_im[q] + w_k_im*temp;
                    temp = x_re[s];
                    x_re[s] = w_2k_re*x_re[s] - w_2k_im*x_im[s];
                    x_im[s] = w_2k_re*x_im[s] + w_2k_im*temp;
                    temp = x_re[t];
                    x_re[t] = w_3k_re*x_re[t] - w_3k_im*x_im[t];
                    x_im[t] = w_3k_re*x_im[t] + w_3k_im*temp;
                    temp = x_re[r];
                    x_re[r] = w_4k_re*x_re[r] - w_4k_im*x_im[r];
                    x_im[r] = w_4k_re*x_im[r] + w_4k_im*temp;

                    // apply butterfly structure
                    // to calculate x_p and x_q (counterpart).
                    // here we calculate by input multiplied with
                    // FFT_3_Matrix(multiplier on butterfly)
                    temp1_re = x_re[p];
                    temp2_re = x_re[q];
                    temp3_re = x_re[s];
                    temp4_re = x_re[t];
                    temp5_re = x_re[r];
                    temp1_im = x_im[p];
                    temp2_im = x_im[q];
                    temp3_im = x_im[s];
                    temp4_im = x_im[t];
                    temp5_im = x_im[r];
                    x_re[p] = temp1_re + temp2_re + temp3_re + temp4_re + temp5_re;
                    x_re[q] = temp1_re + (w_but_re_11*temp2_re - w_but_im_11*temp2_im) \
                                       + (w_but_re_12*temp3_re - w_but_im_12*temp3_im) \
                                       + (w_but_re_13*temp4_re - w_but_im_13*temp4_im) \
                                       + (w_but_re_14*temp5_re - w_but_im_14*temp5_im);
                    x_re[s] = temp1_re + (w_but_re_12*temp2_re - w_but_im_12*temp2_im) \
                                       + (w_but_re_22*temp3_re - w_but_im_22*temp3_im) \
                                       + (w_but_re_23*temp4_re - w_but_im_23*temp4_im) \
                                       + (w_but_re_24*temp5_re - w_but_im_24*temp5_im);
                    x_re[t] = temp1_re + (w_but_re_13*temp2_re - w_but_im_13*temp2_im) \
                                       + (w_but_re_23*temp3_re - w_but_im_23*temp3_im) \
                                       + (w_but_re_33*temp4_re - w_but_im_33*temp4_im) \
                                       + (w_but_re_34*temp5_re - w_but_im_34*temp5_im);
                    x_re[r] = temp1_re + (w_but_re_14*temp2_re - w_but_im_14*temp2_im) \
                                       + (w_but_re_24*temp3_re - w_but_im_24*temp3_im) \
                                       + (w_but_re_34*temp4_re - w_but_im_34*temp4_im) \
                                       + (w_but_re_44*temp5_re - w_but_im_44*temp5_im);


                    x_im[p] = temp1_im + temp2_im + temp3_im + temp4_im + temp5_im;
                    x_im[q] = temp1_im + (w_but_re_11*temp2_im + w_but_im_11*temp2_re) \
                                       + (w_but_re_12*temp3_im + w_but_im_12*temp3_re) \
                                       + (w_but_re_13*temp4_im + w_but_im_13*temp4_re) \
                                       + (w_but_re_14*temp5_im + w_but_im_14*temp5_re);
                    x_im[s] = temp1_im + (w_but_re_12*temp2_im + w_but_im_12*temp2_re) \
                                       + (w_but_re_22*temp3_im + w_but_im_22*temp3_re) \
                                       + (w_but_re_23*temp4_im + w_but_im_23*temp4_re) \
                                       + (w_but_re_24*temp5_im + w_but_im_24*temp5_re);
                    x_im[t] = temp1_im + (w_but_re_13*temp2_im + w_but_im_13*temp2_re) \
                                       + (w_but_re_23*temp3_im + w_but_im_23*temp3_re) \
                                       + (w_but_re_33*temp4_im + w_but_im_33*temp4_re) \
                                       + (w_but_re_34*temp5_im + w_but_im_34*temp5_re);
                    x_im[r] = temp1_im + (w_but_re_14*temp2_im + w_but_im_14*temp2_re) \
                                       + (w_but_re_24*temp3_im + w_but_im_24*temp3_re) \
                                       + (w_but_re_34*temp4_im + w_but_im_34*temp4_re) \
                                       + (w_but_re_44*temp5_im + w_but_im_44*temp5_re);
                }
                // calculate multiplier of next counterpart (with index k)
                temp = w_k_re;
                w_k_re = w_k_re*w_N_re - w_k_im*w_N_im;
                w_k_im = temp  *w_N_im + w_k_im*w_N_re;
                temp = w_2k_re;
                w_2k_re = w_2k_re*w_N_2_re - w_2k_im*w_N_2_im;
                w_2k_im = temp   *w_N_2_im + w_2k_im*w_N_2_re;
                temp = w_3k_re;
                w_3k_re = w_3k_re*w_N_3_re - w_3k_im*w_N_3_im;
                w_3k_im = temp   *w_N_3_im + w_3k_im*w_N_3_re;
                temp = w_4k_re;
                w_4k_re = w_4k_re*w_N_4_re - w_4k_im*w_N_4_im;
                w_4k_im = temp   *w_N_4_im + w_4k_im*w_N_4_re;
            }
        }
    }
}



/// butterfly structure to recombine these input
void butterfly3(double *x_re, double *x_im, int N_st, int N_end, int N){
	int base = 3;
    int m;  // m is third of number in each group
            // it's also the difference between z and u
    int k;  // index in RHS of butterfly (output) (from 0 to m)
            // the rest of m/2 is counterpart of the first m/2
    int p;  // the first index in LHS of butterfly (input1)
    int q;  // the second index in LHS of butterfly (input2)
    int s;  // the third index in LHS of butterfly (input3)

    double w_but_re_11, w_but_re_12, w_but_re_22;
    double w_but_im_11, w_but_im_12, w_but_im_22;
    int i, j;
	int N_thre = omp_get_max_threads();

    w_but_re_11 =  cos((2.0D*M_PI/base));
    w_but_re_12 =  cos((2.0D*M_PI/base)*2.0D);
    w_but_re_22 =  cos((2.0D*M_PI/base)*4.0D);
    w_but_im_11 =  -sin((2.0D*M_PI/base));
    w_but_im_12 =  -sin((2.0D*M_PI/base)*2.0D);
    w_but_im_22 =  -sin((2.0D*M_PI/base)*4.0D);



    // loop for each steps of butterfly (step no.)
    for (m=N_st; m<N_end; m*=base){

		#pragma omp parallel num_threads(N_thre) private(k,p,q,s)
		{
			int id_t = omp_get_thread_num();
			double temp, temp1_re, temp2_re, temp3_re;   // for temporary storage of number
            double temp1_im, temp2_im, temp3_im;	// 1 for p, 2 for q, 3 for s
			// Calculate multiplier of counterpart
	        double w_k_re = cos(2.0D*M_PI/(m*base)*id_t);       // Re(W_{3^m}^id_t)
        	double w_k_im = -sin(2.0D*M_PI/(m*base)*id_t);      // Im(W_{3^m}^id_t)
	        double w_2k_re = w_k_re*w_k_re - w_k_im*w_k_im;		// Re(W_{3^m}^2*id_t)
	        double w_2k_im = 2.0D*w_k_re*w_k_im;             	// Re(W_{3^m}^2*id_t)
	        double w_N_re =  cos(2.0D*M_PI/(m*base)*N_thre); 	// Re(W_{3^m}^N_thre)
	        double w_N_im = -sin(2.0D*M_PI/(m*base)*N_thre); 	// Im(W_{3^m}^N_thre)
	        double w_N_2_re = w_N_re*w_N_re - w_N_im*w_N_im;  	// Re(W_{3^m}^{2*N_thre})
	        double w_N_2_im = 2.0D*w_N_re*w_N_im;  				// Im(W_{3^m}^{2*N_thre})

	        // loop for each output in a group (output no.)
	        for (k=id_t; k<m; k+=N_thre){
	            // loop for each group (group no.)
	            for (p=k; p<N; p+=base*m){
	                // Calculate output of butterfly
	                // find index of counterpart q
	                q = p + m;
	                s = q + m;
	                // apply multiplier of counterpart
	                // say, multiply W_{2^m}^k on x[q]
	                temp = x_re[q];
	                x_re[q] = w_k_re*x_re[q] - w_k_im*x_im[q];
	                x_im[q] = w_k_re*x_im[q] + w_k_im*temp;
	                temp = x_re[s];
	                x_re[s] = w_2k_re*x_re[s] - w_2k_im*x_im[s];
	                x_im[s] = w_2k_re*x_im[s] + w_2k_im*temp;

	                // apply butterfly structure
	                // to calculate x_p and x_q (counterpart).
	                // here we calculate by input multiplied with
	                // FFT_3_Matrix(multiplier on butterfly)
	                temp1_re = x_re[p];
	                temp2_re = x_re[q];
	                temp3_re = x_re[s];
	                temp1_im = x_im[p];
	                temp2_im = x_im[q];
	                temp3_im = x_im[s];
	                x_re[p] = temp1_re + temp2_re + temp3_re;
	                x_re[q] = temp1_re + (w_but_re_11*temp2_re - w_but_im_11*temp2_im) \
									   + (w_but_re_12*temp3_re - w_but_im_12*temp3_im);
	                x_re[s] = temp1_re + (w_but_re_12*temp2_re - w_but_im_12*temp2_im) \
									   + (w_but_re_22*temp3_re - w_but_im_22*temp3_im);

	                x_im[p] = temp1_im + temp2_im + temp3_im;
	                x_im[q] = temp1_im + (w_but_re_11*temp2_im + w_but_im_11*temp2_re) \
									   + (w_but_re_12*temp3_im + w_but_im_12*temp3_re);
	                x_im[s] = temp1_im + (w_but_re_12*temp2_im + w_but_im_12*temp2_re) \
									   + (w_but_re_22*temp3_im + w_but_im_22*temp3_re);
	            }
	            // calculate multiplier of next counterpart (with index k)
	            temp = w_k_re;
	            w_k_re = w_k_re*w_N_re - w_k_im*w_N_im;
	            w_k_im = temp  *w_N_im + w_k_im*w_N_re;
	            temp = w_2k_re;
	            w_2k_re = w_2k_re*w_N_2_re - w_2k_im*w_N_2_im;
	            w_2k_im = temp   *w_N_2_im + w_2k_im*w_N_2_re;
	        }
		}
    }
}



/// butterfly structure to recombine these input
void butterfly2(double *x_re, double *x_im, int N_st, int N_end, int N){
    int base = 2;
	int m;  // m is half of number in each group
            // it's also the difference between z and u
    int k;  // index in RHS of butterfly (output) (from 0 to m)
            // the rest of m/2 is counterpart of the first m/2
    int p;  // the first index in LHS of butterfly (input1)
    int q;  // the second index in LHS of butterfly (input2)
	int N_thre = omp_get_max_threads();


    // loop for each steps of butterfly (step no.)
    for (m=N_st; m<N_end; m*=base){

		// parallel the task
		#pragma omp parallel num_threads(N_thre) private(k,p,q)
		{
			int id_t = omp_get_thread_num();
			double temp;   // for temporary storage of number

			// Calculate multiplier of counterpart
			double w_re = cos(M_PI/m*id_t);       // Re(W_{2^m}^id_t), multiplier of first counterpart
        	double w_im = -sin(M_PI/m*id_t);      // Im(W_{2^m}^id_t)
			double w_N_re = cos(M_PI/m*N_thre);   // Re(W_{2^m}^N_thread)
	        double w_N_im = -sin(M_PI/m*N_thre);  // Im(W_{2^m}^N_thread)

			// loop for each output in a group (output no.)
			for (k=id_t; k<m; k+=N_thre){
	            // loop for each group (group no.)
	            for (p=k; p<N; p+=base*m){
	                // Calculate output of butterfly
	                // find index of counterpart q
	                q = p + m;
	                // apply multiplier of counterpart
	                // say, multiply W_{2^m}^k on x[q]
	                temp = x_re[q];
	                x_re[q] = w_re*x_re[q] - w_im*x_im[q];
	                x_im[q] = w_re*x_im[q] + w_im*temp;

	                // apply butterfly structure
	                // to calculate x_p and x_q (counterpart).
	                // here we calculate by input multiplied with
	                // FFT_2_Matrix(multiplier on butterfly)
	                // ,which is [1 1; 1 -1]
	                temp = x_re[p];
	                x_re[p] = x_re[p] + x_re[q];
	                x_re[q] = temp    - x_re[q];
	                temp = x_im[p];
	                x_im[p] = x_im[p] + x_im[q];
	                x_im[q] = temp    - x_im[q];
	            }
	            // calculate multiplier of next counterpart (with index k)
	            temp = w_re;
	            w_re = w_re*w_N_re - w_im*w_N_im;
	            w_im = temp*w_N_im + w_im*w_N_re;
	        }
		}
    }
}
