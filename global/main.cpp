#include <iostream>
#include "utils.h"
#include <string>
#include <stdio.h>

#define CONV_KERNEL_SIZE 9
//Declaramos las funciones de kernel
void rgba_to_gray(uchar4 * const d_rgbaImage,
                  unsigned char* const d_grayImage,
                  size_t rows,
                  size_t cols);

void sobel_filter(unsigned char* const d_inputImage,
                  unsigned char* const d_outputImage,
                  unsigned int maskWidth,
                  char *M,
                  size_t rows,
                  size_t cols);

//Se incluyen las definiciones del fichero
#include "preprocess.cpp"

//Declaramos las funciones que cargan el kernel de convolucion
void loadConvolutionKernel(char option,char *conv_kernel){
  switch(option){
    case 'b'://Borde
      conv_kernel[0] = 0;
      conv_kernel[1] = 1;
      conv_kernel[2] = 0;
      conv_kernel[3] = 1;
      conv_kernel[4] = -4;
      conv_kernel[5] = 1;
      conv_kernel[6] = 0;
      conv_kernel[7] = 1;
      conv_kernel[8] = 0;
      break;
    default:
      cerr << "Kernel de convolucion indefinido" <<endl;
      return;
  }
  return;
}

int main(int argc,char** argv){
  string input_file;
  string output_file;

  //Imagenes
  uchar4 *h_rgbaImage, *d_rgbaImage;
  unsigned char *h_grayImage, *d_grayImage;
  unsigned char *h_edgeImage, *d_edgeImage;

  //Matriz de convolucion
  char* h_convolutionKernel = (char*)malloc(CONV_KERNEL_SIZE*sizeof(char));
  char* d_convolutionKernel;

  switch (argc) {
    case 1:
      cerr << "No se especifico ningun nombre de fichero" <<endl;
      exit(1);
      break;
    case 2:
      input_file = string(argv[1]);
      output_file = "output.png";
      break;
    default:
      cerr << "Demasiados parametros" <<endl;
      exit(1);
  }

  loadConvolutionKernel('b',h_convolutionKernel);

  //Cargamos la imagen y preparamos los punteros de entrada y salida
  grayscale_preProcess( &h_rgbaImage,
              &h_grayImage,
              &d_rgbaImage,
              &d_grayImage,
              input_file);

  size_t numPixels = imageRGBA.rows * imageRGBA.cols;

  //Invocamos al kernel
  rgba_to_gray( d_rgbaImage,
                d_grayImage,
                imageRGBA.rows,
                imageRGBA.cols);


  checkCudaErrors(
    cudaMemcpy(h_grayImage,d_grayImage,sizeof(unsigned char)*numPixels,cudaMemcpyDeviceToHost)
  );

  //Liberamos memoria
  cudaFree(d_rgbaImage__);

  //Aqui va la convolucion
  sobel_preProcess( &h_edgeImage,
                    &d_edgeImage,
                    &h_convolutionKernel,
                    &d_convolutionKernel);

  sobel_filter(     d_grayImage,
                    d_edgeImage,
                    sqrt(CONV_KERNEL_SIZE),
                    d_convolutionKernel,
                    imageRGBA.rows,
                    imageRGBA.cols);

  checkCudaErrors(
    cudaMemcpy(h_edgeImage,d_edgeImage,sizeof(unsigned char)*numPixels,cudaMemcpyDeviceToHost)
  );

  //Imagen de salida
  Mat output( imageRGBA.rows,
              imageRGBA.cols,
              CV_8UC1,
              (void*)h_edgeImage);

  namedWindow("Display Window", WINDOW_AUTOSIZE);//Creamos una ventana para mostrar la imagen

  //Mostramos los resultados obtenidos
  imshow("Display Window",output);//Mostramos la imagen
  cvWaitKey(0);
  cvDestroyWindow("Display Window");

  imwrite(output_file.c_str(),output);

  //Liberamos memoria
  cudaFree(d_convolutionKernel);
  cudaFree(d_grayImage__);
  cudaFree(d_edgeImage__);

  return 0;
}