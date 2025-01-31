#ifndef AFNI_XML_IO_H
#define AFNI_XML_IO_H


#include <stdio.h>
#include "afni_xml.h"

/* ----------------------------------------------------------------------
   CIFTI XML structure:

      CIFTI
            attr = Version
         Matrix
            MetaData
               MD
                  Name [text]
                  Value [text]
            MatrixIndicesMap
                  attr = ApliesToMatrixDimension,
                         IndicesMapToDataType [int = intent code],
                         NumberOfSeriesPoints, SeriesExponenet,
                         SeriesStart, SeriesStep, SeriesUnit [string??]
               NamedMap
                  MetaData
                  LabelTable
                     Label [text - name of label]
                        attr = Key[int], Red, Green, Blue, Alpha [f 0..1]
                  MapName [text - name of map]
               Surface
                  attr = BrainStructure, SurfaceNumberOfVertices [int64_t]
               Parcel
                  attr = Name [text]
                  Vertices [int64_t (of unknown length?)]
                     attr = BrainStructure
                  VoxelIndicesIJK
               Volume
                  attr = VolumeDimensions [int64_t,int64_t,int64_t]
                  TransformationMatrixVoxelIndicesIJKtoXYZ
                     [text - 16 x double xform matrix (row major)]
                     attr = MeterExponent [int - power of 10]
               BrainModel
                  attr = IndexOffset [int], IndexCount [int64_t],
                         ModelType[??], BrainStructure[??],
                         SurfaceNumberOfVertices
                  VoxelIndicesIJK [int64_t triples (see IndexCount)]
                     * might convert IJK to just Indices via VolumeDimensions
                       (this is only for volume, Surface gives just indices)
                  VertexIndices [int64_t (see IndexCount)]

    - convert as in SUMA_Create_Fake_CIFTI()
 * ----------------------------------------------------------------------*/

#ifndef CIF_API
   #if defined(_WIN32) || defined(__CYGWIN__)
      #if defined(CIFTI_BUILD_SHARED)
      #ifdef __GNUC__
         #define CIF_API __attribute__ ((dllexport))
      #else
         #define CIF_API __declspec( dllexport )
      #endif
      #elif defined(CIFTI_USE_SHARED)
      #ifdef __GNUC__
         #define CIF_API __attribute__ ((dllimport))
      #else
         #define CIF_API __declspec( dllimport )
      #endif
      #else
      #define CIF_API
      #endif
   #elif (defined(__GNUC__) && __GNUC__ >= 4) || defined(__clang__)
      #define CIF_API __attribute__ ((visibility ("default")))
   #else
      #define CIF_API
   #endif
#endif

/* --------------------------- structures --------------------------------- */



/* --------------------------- prototypes --------------------------------- */

CIF_API int axio_read_cifti_file(const char * fname, int get_ndata,
                                 nifti_image ** nim_out, afni_xml_t ** ax_out);

CIF_API afni_xml_t * axio_cifti_from_ext(nifti_image * nim);
CIF_API afni_xml_t * axio_read_buf (const char * buf, int64_t blen);
CIF_API afni_xml_t * axio_read_file(const char * fname);

CIF_API afni_xml_t * axio_find_map_name(afni_xml_t * ax, const char * name, int maxd);

CIF_API int axio_text_to_binary (afni_xml_t * ax);
CIF_API int axio_num_tokens     (const char * str, int64_t maxlen);

CIF_API int axio_show_cifti_summary(FILE * fp, char * mesg, afni_xml_t * ax, int verb);
CIF_API int axio_show_mim_summary(FILE * fp, const char * mesg, afni_xml_t * ax, int verb);
CIF_API int axio_show_attrs(FILE * fp, afni_xml_t * ax, int indent);

#endif /* AFNI_XML_IO_H */
