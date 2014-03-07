/*********************************************************************
 *  get Feature Vector From Model Edges                              *
 *                                                                   *
 *  Hugues Fontenelle, 2014                                          *
 *  The Intevention Centre, Oslo University Hospital, Norway         *
 *********************************************************************/

/*
	Date 25-02-2014

	BUG
	datasource -> problems with nrrd values / coordinates

	physical points corrects?
	DICOM coordinates corrects?
	normals correct?
*/

#include <iostream>

// ITK 
#include "itkImage.h"
#include "itkImageFileReader.h"

// VTK
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkPolyDataReader.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h> 
#include <vtkVector.h>

// timer
#include <time.h>  

#define DEBUG
#define TIMER

////////////////////////////////////////////////////////////////////////////////
int main( int argc, char *argv[] )
{
#ifdef TIMER
	// start timer
	double seconds=0;
	time_t timer1, timer2;
	time(&timer1);  	/* get current time  */
#endif

	////////////////////
	// Verify command line arguments
	if( argc < 3 )
	{
		std::cerr << "Usage: " << std::endl;
		std::cerr << argv[0] << " inputImageFile(*.nrrd) inputModelFile(*.vtk)" << std::endl;
		return EXIT_FAILURE;
	}

	typedef float         InputPixelType;
	const   unsigned int          Dimension = 3;
	typedef itk::Image< InputPixelType, Dimension >    ImageType;
	typedef itk::ImageFileReader< ImageType >   ImageReaderType;
 
	////////////////////
	// read the image 
	ImageReaderType::Pointer imageReader = ImageReaderType::New();
	imageReader->SetFileName( argv[1]  );
	try
	{
		imageReader->Update();
	}
	catch( itk::ExceptionObject & excp )
	{
		std::cerr << "Error during imageReader->Update() " << std::endl;
		std::cerr << excp << std::endl;
		return EXIT_FAILURE;
	}
	ImageType::Pointer image = imageReader->GetOutput();

	////////////////////
	// read the vtk model
	vtkSmartPointer<vtkPolyData> model = vtkSmartPointer<vtkPolyData>::New();;
	vtkSmartPointer<vtkPolyDataReader> vtkReader = vtkSmartPointer<vtkPolyDataReader>::New();
	vtkReader->SetFileName( argv[2] );
	try
	{
		vtkReader->Update();
	}
	catch( itk::ExceptionObject & excp )
	{
		std::cerr << "Error during vtkReader->Update() " << std::endl;
		std::cerr << excp << std::endl;
		return EXIT_FAILURE;
	}
	model->DeepCopy(vtkReader->GetOutput());
	//model = vtkReader->GetOutput();

	////////////////////
	// get physical coordinates from VTK model
	vtkIdType no_of_points = model->GetNumberOfPoints(); 
	typedef itk::Point< double, ImageType::ImageDimension > PointType;
	PointType point;
	typedef itk::Index< ImageType::ImageDimension > IndexType;
	IndexType index;
	InputPixelType pixelValue;
	double pt[3];
	double acc = 0;
	for (int it=0; it < no_of_points; it++) 
	{
		model->GetPoint(it, pt);
		point = pt;

		bool isPointInImage = image->TransformPhysicalPointToIndex( point, index);
		if (isPointInImage)
		{
			pixelValue = image->GetPixel( index ); 
			acc += pixelValue;
		}

	}
	acc /= no_of_points;
#ifdef DEBUG
	std::cout << std::endl << "-- DEBUG points --" << std::endl;
	std::cout << "Number of points: " << no_of_points << std::endl;
	for (int it=0; it < 6; it+=1) 
	{	
		model->GetPoint(it, pt);
		point = pt;
		//	std::cout << "point " << point << std::endl;
		bool isPointInImage = image->TransformPhysicalPointToIndex( point, index);
		if (isPointInImage)
		{
			pixelValue = image->GetPixel( index ); 
			std::cout << "Pt " << it << " (" << pt[0] << "," << pt[1] << "," << pt[2] << ")";
			std::cout << " --> index " << index;
			std::cout << " --> value " << pixelValue << std::endl;
		}
	}
	std::cout << "Mean value = " << acc << std::endl;
#endif

	////////////////////
	// Find normals
	//@BUG: should use face normals?!?
	vtkSmartPointer<vtkFloatArray> pointNormals = 
		vtkFloatArray::SafeDownCast(model->GetPointData()->GetNormals());

#ifdef DEBUG
	std::cout << std::endl << "-- DEBUG normals --" << std::endl;
	std::cout << "Normals: " << std::endl;
	double pN[3];
	for(unsigned int it = 0; it < 6; it+=1)
	{		
		pointNormals->GetTuple(it, pN);
		std::cout << "Point normal " << it << ": " << pN[0] << " " << pN[1] << " " << pN[2] << std::endl;
	}
#endif

	////////////////////
	// Find DICOM spacing 
	const ImageType::SpacingType& inputSpacing = image->GetSpacing();
#ifdef DEBUG
	std::cout << std::endl << "-- DEBUG spacing --" << std::endl;
	std::cout << "Image Spacing = " << inputSpacing[0] << ","
		<< inputSpacing[1] << ","
		<< inputSpacing[2] << std::endl;
#endif
	
	////////////////////
	// Find values along normal 
	typedef itk::Vector< double, ImageType::ImageDimension > NormalType;
	NormalType normal;
	double accVector[7] = {0,0,0,0,0,0,0};
	for (int it=0; it < no_of_points; it++) 
	{
		model->GetPoint(it, pt);
		pointNormals->GetTuple(it, pN);
		point = pt;
		normal = pN;
		for (int offset=-3; offset <=3; offset++)
		{
			bool isPointInImage = image->TransformPhysicalPointToIndex( point+(normal*offset)*inputSpacing[0], index);
			pixelValue = image->GetPixel( index ); 
			accVector[offset+3] += pixelValue;
		}
	}
	for (int it=0; it<7; it++)
		accVector[it] /= no_of_points;

#ifdef DEBUG
	std::cout << std::endl << "-- DEBUG means --" << std::endl;
	std::cout << "Values along normal = ";
	for (int it=0; it<7; it++)
		std::cout << accVector[it] << " ";
	std::cout << std::endl;
#endif


#ifdef TIMER
	// stop timer
	time(&timer2);
	seconds = difftime(timer2,timer1);
	std::cout << std::endl << seconds << " seconds elapsed." << std::endl;
#endif

  return EXIT_SUCCESS;
}
