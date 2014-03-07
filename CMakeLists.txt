cmake_minimum_required(VERSION 2.8)
 
project(FeatureEdge)
 
find_package(ITK 4 REQUIRED)
include(${ITK_USE_FILE})

find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

set(LIBS ${ITK_LIBRARIES} ${VTK_LIBRARIES} )

add_executable(getFeatureVectorFromModelEdges getFeatureVectorFromModelEdges.cxx)
target_link_libraries(getFeatureVectorFromModelEdges ${LIBS})

add_executable(PolyDataExtractNormals  PolyDataExtractNormals.cxx)
target_link_libraries(PolyDataExtractNormals  ${VTK_LIBRARIES})