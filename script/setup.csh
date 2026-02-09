#!/bin/tcsh
# Setup script for ROOT 6 environment with KFParticle support
# This script sets up the environment modules and loads star-env package
# which includes ROOT 6.24.06, KFParticle, GenFit, Eigen, Vc, etc.
#
# Usage: source setup.sh

# Setup Spack environment (for csh/tcsh)
source /cvmfs/star.sdcc.bnl.gov/star-spack/setup.csh

# Setup Spack environment modules for linux-rhel7-x86_64 (64-bit)
module use /cvmfs/star.sdcc.bnl.gov/star-spack/spack/share/spack/modules/linux-rhel7-x86

# Load star-env bundle package with ROOT 6.24.06
# This automatically includes GenFit, Eigen, Vc, and other dependencies
module load root-6.24.06
module load star-env-0.3.0-root-6.24.06

# Explicitly load KFParticle module for ROOT 6.24.06
# This ensures KFParticle headers are in ROOT_INCLUDE_PATH
module load kfparticle-1.1-root-6.24.06

# Display environment information
echo "========================================="
echo "ROOT 6 Environment Setup Complete"
echo "========================================="
echo "ROOTSYS: $ROOTSYS"
echo "ROOT_VERSION: $ROOT_VERSION"
echo ""
echo "KFParticle and other packages are available via:"
echo "  - ROOT_INCLUDE_PATH: $ROOT_INCLUDE_PATH"
echo ""
echo "To verify, try in ROOT:"
echo "  root -l"
echo "  gSystem->Load(\"libKFParticle\")"
echo "========================================="

