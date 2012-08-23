/*-------------------------------------------------------------------
Copyright 2011 Ravishankar Sundararaman, Kendra Letchworth Weaver

This file is part of JDFTx.

JDFTx is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

JDFTx is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with JDFTx.  If not, see <http://www.gnu.org/licenses/>.
-------------------------------------------------------------------*/

#include <fluid/FluidMixture.h>
#include <core/DataIO.h>
#include <core/string.h>
#include <fluid/IdealGasMonoatomic.h>
#include <fluid/Fex_LJ.h>

void initCavity(size_t i,double nc, const double* nEl, double* phi)
{	
	if(nEl[i]>nc) { phi[i]=1.0;}
	else  { phi[i]=0.0;}
}

void initHardSphere(int i, vector3<> r, const vector3<>& r0, double radius, double height, double* phi)
{	phi[i] = ((r - r0).length() < radius ? height : 0.0);
}



int main(int argc, char** argv)
{
	if(argc != 3)
	{	printf("\tUsage: CavitationLJ <output-file> <binary-density-file>\n"
			"\t\t<output-file>: JDFT1 output file (used to get lattice vectors and sample counts)\n"
			"\t\t<binary-density-file>: Electron-density (JDFT1 format) which is thresholded to form cavity\n");
		return 1;
	}

	ifstream ifs(argv[1]);
	if(!ifs.is_open()) { printf("Could not open '%s' for reading\n", argv[1]); return 1; }

	GridInfo gInfo; double jdft1_nc, jdft1_sigma;

	bool gotR=false, gotS=false, got_nc=false;
	while(!ifs.eof())
	{	string line; getline(ifs, line);
		if(line.substr(0,11)=="jdft1-shape")
		{	istringstream ss(line.substr(11));
			ss >> jdft1_nc >> jdft1_sigma;
			got_nc=true;
		}
	/*	if(line=="R = ")
		{	string lBkt, rBkt;
			ifs >> lBkt >>  gInfo.R(0,0) >> gInfo.R(0,1) >> gInfo.R(0,2) >> rBkt;
			ifs >> lBkt >>  gInfo.R(1,0) >> gInfo.R(1,1) >> gInfo.R(1,2) >> rBkt;
			ifs >> lBkt >>  gInfo.R(2,0) >> gInfo.R(2,1) >> gInfo.R(2,2) >> rBkt;
			gotR=true;
		}
		if(line.substr(0,25)=="Chosen fftbox size, S = [")
		{	istringstream ss(line.substr(25));
			ss >> gInfo.S[0] >> gInfo.S[1] >> gInfo.S[2];
			gotS=true;
		}*/ 
		//above commented out b/c using on jdftx revision 75 output file (R and S specified manually)
		
		gInfo.S = vector3<int>(200, 200, 200); double hgrid=40.0/200;
		gotS=true;
		gInfo.R = Diag(gInfo.S*hgrid);	
		gotR=true;
	}
	if(!got_nc) printf("Could not read critical density for JDFT1 shape (nc) from '%s'\n", argv[1]);
	if(!gotR) printf("Could not read lattice vectors (R[][]) from '%s'\n", argv[1]);
	if(!gotS) printf("Could not read sample counts (S[]) from '%s'\n", argv[1]);
	if(!(got_nc && gotR && gotS)) return 1;

	printf("\nParsed '%s' and found (vectors in rows):\n", argv[1]);
	printf("\tR[0] = [ %lf\t%lf\t%lf ];\n",gInfo.R(0,0), gInfo.R(1,0), gInfo.R(2,0));
	printf("\tR[1] = [ %lf\t%lf\t%lf ];\n",gInfo.R(0,1), gInfo.R(1,1), gInfo.R(2,1));
	printf("\tR[2] = [ %lf\t%lf\t%lf ];\n",gInfo.R(0,2), gInfo.R(1,2), gInfo.R(2,2));
	printf("\tS = [ %d %d %d ];\n\n", gInfo.S[0], gInfo.S[1], gInfo.S[2]);
	gInfo.initialize();

	
	FluidMixture fluidMixture(gInfo, 298*Kelvin);

	//----- Excess functional ----- Lennard Jones fluid w/ THF parameters from
	//Journal of Solution Chemistry Volume 22, Number 3, 211-217, DOI: 10.1007/BF00649244
	//Fex_LJ fex(fluidMixture, 519.0*Kelvin, 5.08*Angstrom, "THF");
	Fex_LJ fex(fluidMixture, 519.0*Kelvin, 5.08*Angstrom, "THF");
	
	//----- Ideal gas -----
	IdealGasMonoatomic idgas(&fex,1.0);               

	double p = 1.01325*Bar;
	printf("pV = %le\n", p*gInfo.detR);
	fluidMixture.setPressure(p);

	//----- Initialize external potential -----
	printf("\nReading electron density from '%s' ... ", argv[1]); fflush(stdout);
	DataRptr nElectronic(DataR::alloc(gInfo));
	loadRawBinary(nElectronic, argv[2]);
	printf("SumCheck: %lf electrons\n\n", integral(nElectronic));

	printf("Initializing cavity with nc = %le ... ", jdft1_nc); fflush(stdout);
	nullToZero(idgas.V, gInfo);
	threadedLoop(initCavity, gInfo.nr, jdft1_nc, nElectronic->data(), idgas.V[0]->data());
	double CavityVolume = integral(idgas.V[0]);	
	double CavityRadius = pow(CavityVolume/(4*M_PI),1.0/3.0);
	printf("Cavity Volume: %lf bohr^3\nCavity Radius: %lf bohr\n\n", CavityVolume, CavityRadius);
	//applyFunc_r(gInfo, initHardSphere, gInfo.R*vector3<>(0.5,0.5,0.5), CavityRadius, 0.1, idgas.V[0]->data());


//	RealKernel gaussian(gInfo);
//	initGaussianKernel(gaussian, sqrt(gInfo.h[0].length_squared()+gInfo.h[1].length_squared()+gInfo.h[2].length_squared()));

//	for (int i=0; i<1; i++) idgas.V[i] = 0.005 * I(gaussian * J(idgas.V[i]));
	fluidMixture.initState(0.15);

	MinimizeParams mp;
	mp.alphaTstart = 3e1;
	mp.nDim = gInfo.nr;
	mp.nIterations=200;
	mp.knormThreshold=1e-11;
	mp.nAlphaAdjustMax=5;
	mp.dirUpdateScheme=MinimizeParams::HestenesStiefel;
	
	puts("Starting CG:");
	TIME("minimize", stdout,
		fluidMixture.minimize(mp);
	);
	
	DataRptrCollection N(1);
	double phiFinal = fluidMixture.getFreeEnergy(&N);
	printf("\nCavitation energy = %le Eh\n\n", phiFinal);

}