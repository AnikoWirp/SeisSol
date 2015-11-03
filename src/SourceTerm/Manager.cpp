/**
 * @file
 * This file is part of SeisSol.
 *
 * @author Carsten Uphoff (c.uphoff AT tum.de, http://www5.in.tum.de/wiki/index.php/Carsten_Uphoff,_M.Sc.)
 *
 * @section LICENSE
 * Copyright (c) 2015, SeisSol Group
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * @section DESCRIPTION
 **/
 
#include "Manager.h"
#include "NRFReader.h"
#include "PointSource.h"

#include <Solver/Interoperability.h>
#include <utils/logger.h>
#include <cstring>

#ifdef USE_MPI
#include <mpi.h>
#endif

#if defined(__AVX__)
#include <immintrin.h>
#endif

extern seissol::Interoperability e_interoperability;

template<typename T>
class index_sort_by_value
{
private:
    T const* value;
public:
    index_sort_by_value(T const* value) : value(value) {}
    inline bool operator()(unsigned i, unsigned j) const {
        return value[i] < value[j];
    }
};

void seissol::sourceterm::findMeshIds(Vector3 const* centres, MeshReader const& mesh, unsigned numSources, uint8_t* contained, unsigned* meshIds)
{
  std::vector<Vertex> const& vertices = mesh.getVertices();
  std::vector<Element> const& elements = mesh.getElements();
  
  memset(contained, 0, numSources * sizeof(uint8_t));
  
  double (*planeEquations)[4][4];
  int err = posix_memalign(reinterpret_cast<void**>(&planeEquations), ALIGNMENT, elements.size() * sizeof(double[4][4]));
  if (err != 0) {
    logError() << "Failed to allocate memory in findMeshIds().";
  }
  for (unsigned elem = 0; elem < elements.size(); ++elem) {
    for (int face = 0; face < 4; ++face) {
      VrtxCoords n, p;
      MeshTools::pointOnPlane(elements[elem], face, vertices, p);
      MeshTools::normal(elements[elem], face, vertices, n);
      
      for (unsigned i = 0; i < 3; ++i) {
        planeEquations[elem][i][face] = n[i];
      }
      planeEquations[elem][3][face] = - MeshTools::dot(n, p);
    }
  }
  
  double (*centres1)[4] = new double[numSources][4];
  for (unsigned source = 0; source < numSources; ++source) {
    centres1[source][0] = centres[source].x;
    centres1[source][1] = centres[source].y;
    centres1[source][2] = centres[source].z;
    centres1[source][3] = 1.0;
  }

/// @TODO Could use the code generator for the following
#ifdef _OPENMP
    #pragma omp parallel for schedule(static)
#endif
  for (unsigned elem = 0; elem < elements.size(); ++elem) {
#if 0 //defined(__AVX__)
      __m256d zero = _mm256_setzero_pd();
      __m256d planeDims[4];
      for (unsigned i = 0; i < 4; ++i) {
        planeDims[i] = _mm256_load_pd(&planeEquations[elem][i][0]);
      }
#endif
    for (unsigned source = 0; source < numSources; ++source) {
      int l_notInside = 0;
#if 0 //defined(__AVX__)
      __m256d result = _mm256_setzero_pd();
      for (unsigned dim = 0; dim < 4; ++dim) {
        result = _mm256_add_pd(result, _mm256_mul_pd(planeDims[dim], _mm256_broadcast_sd(&centres1[source][dim])) );
      }
      // >0 => (2^64)-1 ; <0 = 0
      __m256d inside4 = _mm256_cmp_pd(result, zero, _CMP_GE_OQ);
      l_notInside = _mm256_movemask_pd(inside4);
#else
      double result[4] = { 0.0, 0.0, 0.0, 0.0 };
      for (unsigned dim = 0; dim < 4; ++dim) {
        for (unsigned face = 0; face < 4; ++face) {
          result[face] += planeEquations[elem][dim][face] * centres1[source][dim];
        }
      }
      for (unsigned face = 0; face < 4; ++face) {
        l_notInside += (result[face] >= 0.0) ? 1 : 0;
      }
#endif
      if (l_notInside == 0) {
#ifdef _OPENMP
        #pragma omp critical
        {
#endif
          if (contained[source] != 0) {
             logError() << "source with id " << source << " was already found in a different element!";
          }
          contained[source] = 1;
          meshIds[source] = elem;
#ifdef _OPENMP
        }
#endif
      }
    }
  }
  
  free(planeEquations);
  delete[] centres1;
}

#ifdef USE_MPI
void seissol::sourceterm::cleanDoubles(uint8_t* contained, unsigned numSources)
{
  int myrank;
  int size;
  MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  uint8_t* globalContained = new uint8_t[size * numSources];
  MPI_Allgather(contained, numSources, MPI_UINT8_T, globalContained, numSources, MPI_UINT8_T, MPI_COMM_WORLD);
  
  unsigned cleaned = 0;
  for (unsigned source = 0; source < numSources; ++source) {
    if (contained[source] == 1) {
      for (int rank = 0; rank < myrank; ++rank) {
        if (globalContained[rank * numSources + source] == 1) {
          contained[source] = 0;
          ++cleaned;
          break;
        }
      }
    }
  }
  
  if (cleaned > 0) {
    logInfo(myrank) << "Cleaned " << cleaned << " double occurring sources on rank " << myrank << ".";
  }
  
  delete[] globalContained;
}
#endif

void seissol::sourceterm::transformNRFSourceToInternalSource( Vector3 const&            centre,
                                                              unsigned                  element,
                                                              Subfault const&           subfault,
                                                              Offsets const&            offsets,
                                                              Offsets const&            nextOffsets,
                                                              double *const             sliprates[3],
                                                              seissol::model::Material  material,
                                                              PointSources&             pointSources,
                                                              unsigned                  index )
{
  e_interoperability.computeMInvJInvPhisAtSources( centre.x,
                                                   centre.y,
                                                   centre.z,
                                                   element,
                                                   pointSources.mInvJInvPhisAtSources[index] );
  
  real* faultBasis = pointSources.tensor[index];
  faultBasis[0] = subfault.tan1.x;
  faultBasis[1] = subfault.tan1.y;
  faultBasis[2] = subfault.tan1.z;
  faultBasis[3] = subfault.tan2.x;
  faultBasis[4] = subfault.tan2.y;
  faultBasis[5] = subfault.tan2.z;
  faultBasis[6] = subfault.normal.x;
  faultBasis[7] = subfault.normal.y;
  faultBasis[8] = subfault.normal.z;
  
  double mu = (subfault.mu == 0.0) ? material.mu : subfault.mu;  
  pointSources.muA[index] = mu * subfault.area;
  pointSources.lambdaA[index] = material.lambda * subfault.area;
  
  for (unsigned sr = 0; sr < 3; ++sr) {
    unsigned numSamples = nextOffsets[sr] - offsets[sr];
    double const* samples = (numSamples > 0) ? &sliprates[sr][ offsets[sr] ] : NULL;
    samplesToPiecewiseLinearFunction1D( samples,
                                        numSamples,
                                        subfault.tinit,
                                        subfault.timestep,
                                        &pointSources.slipRates[index][sr] );
  }
}

void seissol::sourceterm::Manager::freeSources()
{
  delete[] cmps;
  delete[] sources;
  cmps = NULL;
  sources = NULL;
}

void seissol::sourceterm::Manager::mapPointSourcesToClusters( unsigned const*               meshIds,
                                                              unsigned                      numberOfSources,
                                                              unsigned const              (*meshToClusters)[2],
                                                              unsigned const*               meshToCopyInterior,
                                                              unsigned const*               copyInteriorToMesh,
                                                              MeshStructure const*          meshStructure,
                                                              unsigned                      numberOfClusters )
{
  cmps = new ClusterMapping[numberOfClusters];
  for (unsigned cluster = 0; cluster < numberOfClusters; ++cluster) {
    cmps[cluster].sources           = new unsigned[numberOfSources];
    cmps[cluster].numberOfSources   = 0;
    cmps[cluster].cellToSources     = NULL;
    cmps[cluster].numberOfMappings  = 0;
  }
  
  unsigned* sortedPointSourceIndex = new unsigned[numberOfSources];
  for (unsigned source = 0; source < numberOfSources; ++source) {
    sortedPointSourceIndex[source] = source;
  }
  std::sort(sortedPointSourceIndex, sortedPointSourceIndex + numberOfSources, index_sort_by_value<unsigned>(meshIds));
  
  // Distribute sources to clusters
  for (unsigned source = 0; source < numberOfSources; ++source) {
    unsigned sortedSource = sortedPointSourceIndex[source];
    unsigned meshId = meshIds[sortedSource];
    unsigned cluster = meshToClusters[meshId][0];
    cmps[cluster].sources[ cmps[cluster].numberOfSources++ ] = sortedSource;
  }  
  delete[] sortedPointSourceIndex;

  // Find cell mappings
  unsigned clusterOffset = 0;
  for (unsigned cluster = 0; cluster < numberOfClusters; ++cluster) {
    ClusterMapping& cm = cmps[cluster];
    // Find the cell offsets for a point source. As a cell has 4 neighbors,
    // the cell might exist up to 4 times in the copy layer.
    CellToPointSourcesMapping* cellToPointSources = new CellToPointSourcesMapping[4 * cm.numberOfSources + 1];

    int mapping = -1;
    unsigned lastMeshId = std::numeric_limits<unsigned>::max();
    // add only the interior layer offsets
    for (unsigned clusterSource = 0; clusterSource < cm.numberOfSources; ++clusterSource) {
      unsigned source = cm.sources[clusterSource];
      unsigned meshId = meshIds[source];
      unsigned cell = meshToClusters[meshId][1];
      // If we have a interior cell
      if (cell >= meshStructure[cluster].numberOfCopyCells) {
        if (lastMeshId == meshId) {
          assert(clusterSource <= cellToPointSources[mapping].pointSourcesOffset + cellToPointSources[mapping].numberOfPointSources);
          ++cellToPointSources[mapping].numberOfPointSources;
        } else {
          lastMeshId = meshId;
          ++mapping;
          cellToPointSources[mapping].copyInteriorOffset = cell;
          cellToPointSources[mapping].pointSourcesOffset = clusterSource;
          cellToPointSources[mapping].numberOfPointSources = 1;
        }
      }
    }
    
    // add the copy layer offsets
    for (unsigned cell = 0; cell < meshStructure[cluster].numberOfCopyCells; ++cell) {
      unsigned cellMeshId = copyInteriorToMesh[cell + clusterOffset];
      assert(mapping < 4 * static_cast<int>(cm.numberOfSources));
      ++mapping;
      cellToPointSources[mapping].numberOfPointSources = 0;
      cellToPointSources[mapping].copyInteriorOffset = cell;
    
      for (unsigned clusterSource = 0; clusterSource < cm.numberOfSources; ++clusterSource) {
        unsigned source = cm.sources[clusterSource];
        unsigned meshId = meshIds[source];
        if (meshId == cellMeshId) {
          if (cellToPointSources[mapping].numberOfPointSources == 0) {
            cellToPointSources[mapping].pointSourcesOffset = clusterSource;
          }
          assert(clusterSource <= cellToPointSources[mapping].pointSourcesOffset + cellToPointSources[mapping].numberOfPointSources);
          ++cellToPointSources[mapping].numberOfPointSources;
        }
      }
        
      if (cellToPointSources[mapping].numberOfPointSources == 0) {
        --mapping;
      }
    }
    
    cm.numberOfMappings = mapping+1;
    
    cm.cellToSources = new CellToPointSourcesMapping[ cm.numberOfMappings ];
    for (unsigned mapping = 0; mapping < cm.numberOfMappings; ++mapping) {
      cm.cellToSources[mapping] = cellToPointSources[mapping];
    }    
    delete[] cellToPointSources;
    clusterOffset += meshStructure[cluster].numberOfCopyCells + meshStructure[cluster].numberOfInteriorCells;
  }
}

void seissol::sourceterm::Manager::loadSourcesFromFSRM( double const*                 momentTensor,
                                                        int                           numberOfSources,
                                                        double const*                 centres,
                                                        double const*                 strikes,
                                                        double const*                 dips,
                                                        double const*                 rakes,
                                                        double const*                 onsets,
                                                        double const*                 areas,
                                                        double                        timestep,
                                                        int                           numberOfSamples,
                                                        double const*                 timeHistories,
                                                        MeshReader const&             mesh,
                                                        CellMaterialData const*       materials,
                                                        unsigned const              (*meshToClusters)[2],
                                                        unsigned const*               meshToCopyInterior,
                                                        unsigned const*               copyInteriorToMesh,
                                                        MeshStructure const*          meshStructure,
                                                        unsigned                      numberOfClusters,
                                                        time_stepping::TimeManager&   timeManager)
{
  freeSources();
  
  int rank;
#ifdef USE_MPI
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
#else
  rank = 0;
#endif
  
  logInfo(rank) << "<--------------------------------------------------------->";
  logInfo(rank) << "<                      Point sources                      >";
  logInfo(rank) << "<--------------------------------------------------------->";

  uint8_t* contained = new uint8_t[numberOfSources];
  unsigned* meshIds = new unsigned[numberOfSources];
  Vector3* centres3 = new Vector3[numberOfSources];
  
  for (int source = 0; source < numberOfSources; ++source) {
    centres3[source].x = centres[3*source];
    centres3[source].y = centres[3*source + 1];
    centres3[source].z = centres[3*source + 2];
  }
  
  logInfo(rank) << "Finding meshIds for point sources...";
  
  findMeshIds(centres3, mesh, numberOfSources, contained, meshIds);

#ifdef USE_MPI
  logInfo(rank) << "Cleaning possible double occurring point sources for MPI...";
  cleanDoubles(contained, numberOfSources);
#endif

  unsigned* originalIndex = new unsigned[numberOfSources];
  unsigned numSources = 0;
  for (int source = 0; source < numberOfSources; ++source) {
    originalIndex[numSources] = source;
    meshIds[numSources] = meshIds[source];
    numSources += contained[source];
  }
  delete[] contained;

  logInfo(rank) << "Mapping point sources to LTS cells...";
  mapPointSourcesToClusters(meshIds, numSources, meshToClusters, meshToCopyInterior, copyInteriorToMesh, meshStructure, numberOfClusters);
  
  real localMomentTensor[3][3];
  for (unsigned i = 0; i < 9; ++i) {
    *(&localMomentTensor[0][0] + i) = momentTensor[i];
  }
  
  sources = new PointSources[numberOfClusters];
  for (unsigned cluster = 0; cluster < numberOfClusters; ++cluster) {
    sources[cluster].mode                  = PointSources::FSRM;
    sources[cluster].numberOfSources       = cmps[cluster].numberOfSources;
    /// \todo allocate aligned or remove ALIGNED_
    sources[cluster].mInvJInvPhisAtSources = new real[cmps[cluster].numberOfSources][NUMBER_OF_ALIGNED_BASIS_FUNCTIONS];
    sources[cluster].tensor                = new real[cmps[cluster].numberOfSources][9];
    sources[cluster].slipRates             = new PiecewiseLinearFunction1D[cmps[cluster].numberOfSources][3];

    for (unsigned clusterSource = 0; clusterSource < cmps[cluster].numberOfSources; ++clusterSource) {
      unsigned sourceIndex = cmps[cluster].sources[clusterSource];
      unsigned fsrmIndex = originalIndex[sourceIndex];
      
      e_interoperability.computeMInvJInvPhisAtSources( centres3[fsrmIndex].x,
                                                       centres3[fsrmIndex].y,
                                                       centres3[fsrmIndex].z,
                                                       meshIds[sourceIndex],
                                                       sources[cluster].mInvJInvPhisAtSources[clusterSource] );

      transformMomentTensor( localMomentTensor,
                             strikes[fsrmIndex],
                             dips[fsrmIndex],
                             rakes[fsrmIndex],
                             sources[cluster].tensor[clusterSource] );
      for (unsigned i = 0; i < 9; ++i) {
        sources[cluster].tensor[clusterSource][i] *= areas[fsrmIndex];
      }
      
      samplesToPiecewiseLinearFunction1D( &timeHistories[fsrmIndex * numberOfSamples],
                                          numberOfSamples,
                                          onsets[fsrmIndex],
                                          timestep,
                                          &sources[cluster].slipRates[clusterSource][0] );
    }
  }
  delete[] originalIndex;
  delete[] meshIds;
  delete[] centres3;
  
  timeManager.setPointSourcesForClusters(cmps, sources);
}

#ifdef USE_NETCDF
void seissol::sourceterm::Manager::loadSourcesFromNRF( char const*                   fileName,
                                                       MeshReader const&             mesh,
                                                       CellMaterialData const*       materials,
                                                       unsigned const              (*meshToClusters)[2],
                                                       unsigned const*               meshToCopyInterior,
                                                       unsigned const*               copyInteriorToMesh,
                                                       MeshStructure const*          meshStructure,
                                                       unsigned                      numberOfClusters,
                                                       time_stepping::TimeManager&   timeManager )
{
  freeSources();
  
  int rank;
#ifdef USE_MPI
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
#else
  rank = 0;
#endif

  logInfo(rank) << "<--------------------------------------------------------->";
  logInfo(rank) << "<                      Point sources                      >";
  logInfo(rank) << "<--------------------------------------------------------->";
  
  logInfo(rank) << "Reading" << fileName;
  NRF nrf;
  readNRF(fileName, nrf);
  
  uint8_t* contained = new uint8_t[nrf.source];
  unsigned* meshIds = new unsigned[nrf.source];
  
  logInfo(rank) << "Finding meshIds for point sources...";
  findMeshIds(nrf.centres, mesh, nrf.source, contained, meshIds);

#ifdef USE_MPI
  logInfo(rank) << "Cleaning possible double occurring point sources for MPI...";
  cleanDoubles(contained, nrf.source);
#endif

  unsigned* originalIndex = new unsigned[nrf.source];
  unsigned numSources = 0;
  for (unsigned source = 0; source < nrf.source; ++source) {
    originalIndex[numSources] = source;
    meshIds[numSources] = meshIds[source];
    numSources += contained[source];
  }
  delete[] contained;

  logInfo(rank) << "Mapping point sources to LTS cells...";
  mapPointSourcesToClusters(meshIds, numSources, meshToClusters, meshToCopyInterior, copyInteriorToMesh, meshStructure, numberOfClusters);
  
  sources = new PointSources[numberOfClusters];
  for (unsigned cluster = 0; cluster < numberOfClusters; ++cluster) {
    sources[cluster].mode                  = PointSources::NRF;
    sources[cluster].numberOfSources       = cmps[cluster].numberOfSources;
    /// \todo allocate aligned or remove ALIGNED_
    sources[cluster].mInvJInvPhisAtSources = new real[cmps[cluster].numberOfSources][NUMBER_OF_ALIGNED_BASIS_FUNCTIONS];
    sources[cluster].tensor                = new real[cmps[cluster].numberOfSources][9];
    sources[cluster].muA                   = new real[cmps[cluster].numberOfSources];
    sources[cluster].lambdaA               = new real[cmps[cluster].numberOfSources];
    sources[cluster].slipRates             = new PiecewiseLinearFunction1D[cmps[cluster].numberOfSources][3];

    for (unsigned clusterSource = 0; clusterSource < cmps[cluster].numberOfSources; ++clusterSource) {
      unsigned sourceIndex = cmps[cluster].sources[clusterSource];
      unsigned nrfIndex = originalIndex[sourceIndex];
      transformNRFSourceToInternalSource( nrf.centres[nrfIndex],
                                          meshIds[sourceIndex],
                                          nrf.subfaults[nrfIndex],
                                          nrf.sroffsets[nrfIndex],
                                          nrf.sroffsets[nrfIndex+1],
                                          nrf.sliprates,
                                          materials[ meshToCopyInterior[ meshIds[sourceIndex] ] ].local,
                                          sources[cluster],
                                          clusterSource );
    }
  }
  delete[] originalIndex;
  delete[] meshIds;
  
  timeManager.setPointSourcesForClusters(cmps, sources);
}
#endif