#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <complex.h>
#include <string.h>
#include <sndfile.h>
#include "fft.h"

double *get_freq(double *freqArray, double *arr, int size, int samplerate)
{
    int i;
    long int N;
    double complex *c_arr = (double complex *)malloc(size * sizeof(double complex));
    double complex *fft_output;
    Frame *output_frames, *peaks;
    int num_peaks;

    for (i = 0; i < size; i++)
    {
        c_arr[i] = arr[i] + 0 * I;
    }
    c_arr = zero_pad_arr(c_arr, size, &N);
    fft_output = FFT(c_arr, N);
    output_frames = (Frame *)malloc(N / 2 * sizeof(Frame));
    for (i = 0; i < N / 2; i++)
    {
        output_frames[i].index = i;
        output_frames[i].energy = creal(fft_output[i]);
        output_frames[i].freq = i * ((double)samplerate / N);
    }
    peaks = find_peaks(output_frames, N / 2, &num_peaks);
    quickSort(peaks, 0, num_peaks - 1);
    for (i = 1; i <= 5; i++)
    {
        freqArray[i - 1] = peaks[num_peaks - i].freq;
    }
    free(c_arr);
    free(fft_output);
    free(output_frames);
    free(peaks);
    return freqArray;
}

double complex *zero_pad_arr(double complex *arr, int size, long int *padded_size)
{
    *padded_size = 1;
    while (*padded_size < size)
    {
        *padded_size *= 2;
    }
    double complex *padded_arr = (double complex *)malloc(*padded_size * sizeof(double complex));
    if (padded_arr == NULL)
    {
        fprintf(stderr, "Memory allocation failed.\n");
        exit(1);
    }
    for (int i = 0; i < size; i++)
    {
        padded_arr[i] = arr[i];
    }
    for (int i = size; i < *padded_size; i++)
    {
        padded_arr[i] = 0.0 + 0.0 * I;
    }
    return padded_arr;
}

// FFT functions
double complex *FFT(double complex *P, int n)
{
    int i;
    double complex *P_e, *P_o, *y_e, *y_o;
    double complex *y = (double complex *)malloc(n * sizeof(double complex));
    ;
    complex w = cexp((2 * M_PI * I) / n);
    if (n == 1)
    {
        double complex *P_new = (double complex *)malloc(sizeof(double complex));
        P_new[0] = P[0];
        return P_new;
    }
    P_e = create_alternating_array(&P[0], n / 2);
    P_o = create_alternating_array(&P[1], n / 2);
    y_e = FFT(P_e, n / 2);
    y_o = FFT(P_o, n / 2);

    for (i = 0; i < n; i++)
    {
        y[i] = 0;
    }
    for (i = 0; i < n / 2; i++)
    {
        y[i] = y_e[i] + cpow(w, i) * y_o[i];
        y[i + n / 2] = y_e[i] - cpow(w, i) * y_o[i];
    }
    // printf("Addresses: %p %p %p %p\n", (void *)P_e, (void *)P_o, (void *)y_e, (void *)y_o);
    free(P_e);
    free(P_o);
    free(y_e);
    free(y_o);
    return y;
}

double complex *create_alternating_array(double complex *start, int size)
{
    int i = 0;
    double complex *pos = start;
    double complex *arr = (double complex *)malloc(size * sizeof(double complex));
    while (i < size)
    {
        arr[i] = pos[0];
        i++;
        pos += 2;
    }
    return arr;
}

// Printing functions
void print_complex(double complex number)
{
    printf("%.10f + %.10fi\n", creal(number), cimag(number));
}

void print_complex_array(double complex *number, int size)
{
    int i = 0;
    double complex *pos = number;
    while (i < size)
    {
        print_complex(pos[0]);
        pos++;
        i++;
    }
}

void print_double_array(double *doubles, int size)
{
    int i = 0;
    double *pos = doubles;
    printf("[");
    while (i < size)
    {
        printf("%lf, ", pos[0]);
        pos++;
        i++;
    }
    printf("]\n");
}

// FFT result Analysis
Frame *find_peaks(Frame *arr, int size, int *num_peaks)
{
    Frame *peaks = (Frame *)malloc(size * sizeof(Frame));
    if (peaks == NULL)
    {
        fprintf(stderr, "Memory allocation failed.\n");
        exit(1);
    }
    *num_peaks = 0;
    for (int i = 1; i < size - 1; i++)
    {
        if (arr[i].energy > arr[i - 1].energy && arr[i].energy > arr[i + 1].energy)
        {
            peaks[*num_peaks].index = arr[i].index;
            peaks[*num_peaks].energy = arr[i].energy;
            peaks[*num_peaks].freq = arr[i].freq;
            (*num_peaks)++;
        }
    }
    peaks = (Frame *)realloc(peaks, (*num_peaks) * sizeof(Frame));
    if (peaks == NULL && *num_peaks > 0)
    {
        fprintf(stderr, "Memory reallocation failed.\n");
        exit(1);
    }
    return peaks;
}

// Sorting (Quicksort)
void swap(Frame *p1, Frame *p2)
{
    Frame temp;
    temp = p1[0];
    p1[0] = p2[0];
    p2[0] = temp;
}

int partition(Frame *arr, int low, int high)
{
    double pivot = arr[high].energy;
    int i = low - 1;
    for (int j = low; j <= high; j++)
    {
        if (arr[j].energy < pivot)
        {
            i++;
            swap(&arr[i], &arr[j]);
        }
    }
    swap(&arr[i + 1], &arr[high]);
    return (i + 1);
}

void quickSort(Frame *arr, int low, int high)
{
    // sorts Frames by value and returns array of indexes in descending value
    if (low < high)
    {
        int pi = partition(arr, low, high);
        quickSort(arr, low, pi - 1);
        quickSort(arr, pi + 1, high);
    }
}

// Testing functions
double complex *generate_test_array(int size)
{
    double complex *arr = malloc(size * sizeof(double complex));
    if (arr == NULL)
    {
        printf("Memory allocation failed\n");
        exit(1);
    }
    for (int i = 0; i < size; i++)
    {
        double complex x = (double complex)i / 10;
        arr[i] = csin(x);
    }
    return arr;
}
