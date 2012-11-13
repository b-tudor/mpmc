#include <mc.h>


void check_ensemble ( system_t * system, int ensemble ) {

	switch(ensemble) {
		case ENSEMBLE_UVT:
			output("INPUT: Grand canonical ensemble\n");
			break;
		case ENSEMBLE_NVT:
			output("INPUT: Canonical ensemble\n");
			break;
		case ENSEMBLE_SURF:
			output("INPUT: Potential energy surface\n");
			break;
		case ENSEMBLE_SURF_FIT:
			output("INPUT: Potential energy surface fitting\n");
			break;
		case ENSEMBLE_NVE:
			output("INPUT: Microcanonical ensemble\n");
			break;
		case ENSEMBLE_TE:
			output("INPUT: Single-point energy calculation\n");
			system->numsteps = 0;
			system->corrtime = 0;
			break;
		case ENSEMBLE_NPT:
			output("INPUT: Isobaric-Isothermal ensemble\n");
			break;
		case ENSEMBLE_REPLAY:
			output("INPUT: Replaying trajectory\n");
			break;
		default:
			error("INPUT: improper ensemble specified\n");
			die(-1);
	}
	
	return;
}

void ensemble_surf_fit_options ( system_t * system ) {

	// Record number of curves for convenient reference
	int nCurves = system->fit_input_list.data.count;

	if( !nCurves ) {
		error( "INPUT: There were no fit_input files specified in the main input file.\n" );
		die(-1);
	}

	if( nCurves < 2 ) {
		error( "INPUT: There were less than two fit_input files specified in the main input file.\n" );
		error( "       A minimum of two fit_input files are required for surface-fit calculations.\n" );
		die(-1);
	}

	return;
}

void ensemble_surf_options ( system_t * system ) {
	
	char linebuf[MAXLINE];
	if(system->surf_max < system->surf_min) {
		error("INPUT: surf_max is greater than surf_min\n");
		die(-1);
	} else {
		sprintf(linebuf, "INPUT: minimum surface coordinate is %.3f\n", system->surf_min);
		output(linebuf);
		sprintf(linebuf, "INPUT: maximum surface coordinate is %.3f\n", system->surf_max);
		output(linebuf);
	}

	if(system->surf_inc <= 0.0) {
		error("INPUT: surf_inc is less than or equal to 0\n");
		die(-1);
	} else {
		sprintf(linebuf, "INPUT: incremental surface displacement coordinate is %.3f\n", system->surf_inc);
		output(linebuf);
	}

	if(!system->surf_preserve && (system->surf_ang <= 0.0)) {
		error("INPUT: surf_ang is less than or equal to 0\n");
		die(-1);
	} else {
		sprintf(linebuf, "INPUT: incremental surface angle coordinate is %.3f\n", system->surf_ang);
		output(linebuf);
	}

	if (system->read_pqr_box_on) {
		error("INPUT: read_pqr_box is not compatible with surf.\n");
		die(-1);
	}

	return;
}

void spectre_options (system_t * system) {
	
	char linebuf[MAXLINE];
	if(system->ensemble != ENSEMBLE_NVT) {
		error("INPUT: SPECTRE algorithm requires canonical ensemble\n");
		die(-1);
	} else {
		output("INPUT: SPECTRE algorithm activated\n");
		sprintf(linebuf, "INPUT: SPECTRE max charge = %.3f\n", system->spectre_max_charge);
		output(linebuf);
		sprintf(linebuf, "INPUT: SPECTRE max target = %.3f\n", system->spectre_max_target);
		output(linebuf);
	}

	return;
}

void feynman_hibbs_options ( system_t * system ) {

	char linebuf[MAXLINE];
	output("INPUT: Feynman-Hibbs effective potential activated\n");

	if(system->feynman_kleinert) {
		output("INPUT: Feynman-Kleinert iteration method activated\n");

		if(!system->rd_anharmonic) {
			error("INPUT: Feynman-Kleinert iteration only implemented for anharmonic oscillator\n");
			die(-1);
		}
	} 
	else {
		switch(system->feynman_hibbs_order) {
			case 2:
				sprintf(linebuf, "INPUT: Feynman-Hibbs second-order quantum correction activated\n");
				output(linebuf);
				break;
			case 4:
				sprintf(linebuf, "INPUT: Feynman-Hibbs fourth-order quantum correction activated\n");
				output(linebuf);
				break;
			default:
				output("INPUT: Feynman-Hibbs order unspecified - defaulting to h^2\n");
				system->feynman_hibbs_order = 2;
				break;
		}
	}
	//if using polarvdw and FH, cavity_autoreject_absolute must be on (otherwise shit blows up)
	if ( (system->polarvdw) && !(system->cavity_autoreject_absolute) ) {
		error("INPUT: cavity_autoreject_absolute must be used with polarvdw + Feynman Hibbs.\n");
		die(-1);
	}

	if ( system->temperature <= 0 ) {
		error("INPUT: feynman_hibbs requires positive temperature.\n");
		die(-1);
	}

	return;
}

void qrot_options(system_t * system) {

	char linebuf[MAXLINE];
	output("INPUT: Quantum rotational eigenspectrum calculation enabled\n");
	if(system->quantum_rotation_B <= 0.0) {
		error("INPUT: invalid quantum rotational constant B specified\n");
		die(-1);
	} else {
		sprintf(linebuf, "INPUT: Quantum rotational constant B = %.3f K (%.3f cm^-1)\n", system->quantum_rotation_B, system->quantum_rotation_B*KB/(100.0*H*C));
		output(linebuf);
	}

	if(system->quantum_rotation_level_max <= 0) {
		error("INPUT: invalid quantum rotation level max\n");
		die(-1);
	} else {
		sprintf(linebuf, "INPUT: Quantum rotation level max = %d\n", system->quantum_rotation_level_max);
		output(linebuf);
	}

	if(system->quantum_rotation_l_max <= 0) {
		error("INPUT: invalid quantum rotation l_max\n");
		die(-1);
	} else {
		sprintf(linebuf, "INPUT: Quantum rotation l_max = %d\n", system->quantum_rotation_l_max);
		output(linebuf);
	}

	if(system->quantum_rotation_level_max > (system->quantum_rotation_l_max+1)*(system->quantum_rotation_l_max+1)) {
		error("INPUT: quantum rotational levels cannot exceed l_max + 1 X l_max +1\n");
		die(-1);
	}

	if(system->quantum_rotation_sum <= 0) {
		error("INPUT: quantum rotational sum for partition function invalid\n");
		die(-1);
	} else {
		sprintf(linebuf, "INPUT: Quantum rotation sum = %d\n", system->quantum_rotation_sum);
		output(linebuf);
	}

	return;
}

void simulated_annealing_options( system_t * system) {

	char linebuf[MAXLINE];
	output("INPUT: Simulated annealing active\n");

	if((system->simulated_annealing_schedule < 0.0) || (system->simulated_annealing_schedule > 1.0)) {
		error("INPUT: invalid simulated annealing temperature schedule specified\n");
		die(-1);
	} else {
		sprintf(linebuf, "INPUT: Simulated annealing temperature schedule = %.3f\n", system->simulated_annealing_schedule);
		output(linebuf);
	}

	if(system->simulated_annealing_target < 0.0) {
		error("INPUT: invalid simulated annealing target specified\n");
		die(-1);
	} else {
		sprintf(linebuf, "INPUT: Simulated annealing target %lfK.", system->simulated_annealing_target);
		output(linebuf);
	}

	return;
}


void polarization_options (system_t * system) {

	char linebuf[MAXLINE];

	output("INPUT: Thole polarization activated\n");

	if(system->cuda) {
		if(!system->polar_iterative) {
			error("INPUT: CUDA GPU acceleration available for iterative Thole only\n");
			die(-1);
		} else if(system->damp_type != DAMPING_EXPONENTIAL) {
			error("INPUT: CUDA GPU accleration available for exponential Thole damping only\n");
			die(-1);
		} else if(!system->polar_max_iter) {
			/* XXX disable for 1 iter testing */
			//error("INPUT: Must set polar_max_iter for CUDA GPU acceleration\n");
			//die(-1);
		} else
			output("INPUT: CUDA GPU Thole SCF solver activated\n");
	}

	if(system->polar_iterative && system->polarizability_tensor) {
		error("INPUT: iterative polarizability tensor method not implemented\n");
		die(-1);
	}

	if(!system->polar_iterative && system->polar_zodid) {
		error("INPUT: ZODID and matrix inversion cannot both be set!\n");
		die(-1);
	}

	if(system->polar_wolf || system->polar_wolf_full) {
		if ( system->polar_wolf )
			output("INPUT: Polar wolf activated. Thole field calculated using wolf method.\n");
		if ( system->polar_wolf_full ) 
			output("INPUT: Full polar wolf treatment activated.\n");
		if ( (system->polar_wolf_alpha >= 0 ) && ( system->polar_wolf_alpha <= 1 ) ) {
			sprintf(linebuf,"INPUT: Polar wolf damping set to %lf. (0 is default)\n", system->polar_wolf_alpha);
			output(linebuf);
		} else {
			error("INPUT: 1 >= polar_wolf_alpha >= 0 is required.\n");
			die(-1);
		}
	}

	if(system->polar_ewald) 
		output("INPUT: Polar ewald activated. Thole field calculated using ewald method.\n");
	
	if(system->polar_ewald_full) {
		output("INPUT: Full ewald polarization activated.\n");
		if(!system->polar_max_iter)	{
			output("INPUT: Setting polar_ewald_full max_iter to default value of 10.\n");
			system->polar_max_iter = 10;
		}
		else if (system->polar_precision) {
			error("INPUT: polar_ewald_full is not compat with polar_precision. Use polar_max_iter instead.\n");
			die(-1);
		}
		else if (system->polar_rrms ) {
			error("INPUT: polar_ewald_full is not compat with polar_rrms. Deal with it.\n");
			die(-1);
		}
	}
	
	if(system->damp_type == DAMPING_LINEAR)
		output("INPUT: Thole linear damping activated\n");
	else if (system->damp_type == DAMPING_OFF)
		output("INPUT: Thole linear damping is OFF\n");
	else if(system->damp_type == DAMPING_EXPONENTIAL)
		output("INPUT: Thole exponential damping activated\n");
	else {
		error("INPUT: Thole damping method not specified\n");
		die(-1);
	}

	if(system->polar_damp <= 0.0) {
		error("INPUT: damping factor must be specified\n");
		die(-1);
	} else {
		sprintf(linebuf, "INPUT: Thole damping parameter is %.4f\n", system->polar_damp);
		output(linebuf);
	}

	if(system->polar_iterative) {

		output("INPUT: Thole iterative solver activated\n");
		if(system->polar_zodid) {
			output("INPUT: ZODID polarization enabled\n");
		}

		if((system->polar_precision > 0.0) && (system->polar_max_iter > 0)) {
			error("INPUT: cannot specify both polar_precision and polar_max_iter, must pick one\n");
			die(-1);
		}
	
		if(system->polar_precision < 0.0) {
			error("INPUT: invalid polarization iterative precision specified\n");
			die(-1);
		} else if(system->polar_precision > 0.0) {
			sprintf(linebuf, "INPUT: Thole iterative precision is %e A*sqrt(KA) (%e D)\n", system->polar_precision, system->polar_precision/DEBYE2SKA);
			output(linebuf);
		} else {
			sprintf(linebuf, "INPUT: using polar max SCF iterations = %d\n", system->polar_max_iter);
			output(linebuf);
		}

		if(system->polar_precision > 0.0 || system->polar_rrms) 
			output("INPUT: polar_rrms activated. Dipole rrms will be reported.\n");
		
		if(system->polar_sor && system->polar_esor) {
			error("INPUT: cannot specify both SOR and ESOR SCF methods\n");
			die(-1);
		}

		if(system->polar_sor) output("INPUT: SOR SCF scheme active\n");
		else if(system->polar_esor) output("INPUT: ESOR SCF scheme active\n");

		if(system->polar_gamma < 0.0) {
			error("INPUT: invalid Pre-cond/SOR/ESOR gamma set\n");
			die(-1);
		} else {
			sprintf(linebuf, "INPUT: Pre-cond/SOR/ESOR gamma = %.3f\n", system->polar_gamma);
			output(linebuf);
		}

		if(system->polar_gs && system->polar_gs_ranked) {
			error("INPUT: both polar_gs and polar_gs_ranked cannot be set\n");
			die(-1);
		}

		if(system->polar_gs)
			output("INPUT: Gauss-Seidel iteration scheme active\n");
		else if(system->polar_gs_ranked)
			output("INPUT: Gauss-Seidel Ranked iteration scheme active\n");

		if(system->polar_palmo) output("INPUT: Polarization energy of Palmo and Krimm enabled\n");

	} 
	else {
		output("INPUT: Matrix polarization activated\n");
		if(system->polarizability_tensor)
			output("INPUT: Polarizability tensor calculation activated\n");
	}

	if(system->polar_self) output("INPUT: Polarization self-induction is active\n");

	if(system->polarvdw) {
		output("INPUT: polarvdw (coupled-dipole van der Waals) activated\n");
		if(system->feynman_hibbs) {
			if(system->vdw_fh_2be)
				output("INPUT: two-body-expansion feynman-hibbs for polarvdw is active\n");
			else
				output("INPUT: polarvdw feynman-hibbs will be calculated using MPFD\n");
		}
	}

	return;
}


void write_pbc_info ( system_t * system ) {
	char linebuf[MAXLINE];

	sprintf(linebuf, "INPUT: pbc_cutoff set to %.5f A\n", system->pbc->cutoff);
	output(linebuf);
	sprintf(linebuf, "INPUT: basis vector 1 = %.5f %.5f %.5f\n", 
		system->pbc->basis[0][0], system->pbc->basis[0][1], system->pbc->basis[0][2]);
	output(linebuf);
	sprintf(linebuf, "INPUT: basis vector 2 = %.5f %.5f %.5f\n", 
		system->pbc->basis[1][0], system->pbc->basis[1][1], system->pbc->basis[1][2]);
	output(linebuf);
	sprintf(linebuf, "INPUT: basis vector 3 = %.5f %.5f %.5f\n", 
		system->pbc->basis[2][0], system->pbc->basis[2][1], system->pbc->basis[2][2]);
	output(linebuf);
	sprintf(linebuf, "INPUT: unit cell volume = %.3f A^3 (cutoff = %.3f A)\n", 
		system->pbc->volume, system->pbc->cutoff);
	output(linebuf);
	sprintf(linebuf, "INPUT: recip basis vector 1 = %.5f %.5f %.5f\n", 
		system->pbc->reciprocal_basis[0][0], system->pbc->reciprocal_basis[0][1], system->pbc->reciprocal_basis[0][2]);
	output(linebuf);
	sprintf(linebuf, "INPUT: recip basis vector 2 = %.5f %.5f %.5f\n", 
		system->pbc->reciprocal_basis[1][0], system->pbc->reciprocal_basis[1][1], system->pbc->reciprocal_basis[1][2]);
	output(linebuf);
	sprintf(linebuf, "INPUT: recip basis vector 3 = %.5f %.5f %.5f\n", 
		system->pbc->reciprocal_basis[2][0], system->pbc->reciprocal_basis[2][1], system->pbc->reciprocal_basis[2][2]);
	output(linebuf);

	return;
}

void ensemble_te_options(system_t * system) {

	//nothing to do

	return;
}

void ensemble_replay_options(system_t * system) {
	char linebuf[MAXLINE];

	if (system->calc_pressure) {
		output("INPUT: pressure calculator is on\n");
		if ( system->calc_pressure_dv == 0 ) {
			sprintf(linebuf, "INPUT: calc_pressure is on, but calc_pressure_dv is not set\n");
			error(linebuf);
			die(-1);
		}
		sprintf(linebuf, "INPUT: pressure calculator dV = %lf\n", system->calc_pressure_dv);
		output(linebuf);
		if ( system->temperature <= 0 ) {
			sprintf(linebuf, "INPUT: pressure calculator requires non-zero temperature\n");
			output(linebuf);
			die(-1);
		}
	}

	return;
}

void mc_options (system_t * system) {
	int i;
	char linebuf[MAXLINE];

	if (system->numsteps < 1) {
		error("INPUT: improper numsteps specified\n");
		die(-1);
	} else {
		sprintf(linebuf, "INPUT: each core performing %d simulation steps\n", system->numsteps);
		output(linebuf);
	}

	if (system->corrtime < 1)  {
		error("INPUT: improper corrtime specified\n");
		die(-1);
	} else {
		sprintf(linebuf, "INPUT: system correlation time is %d steps\n", system->corrtime);
		output(linebuf);
	}

	if(system->ensemble != ENSEMBLE_NVE) {
		if(system->temperature <= 0.0) {
			error("INPUT: invalid temperature specified\n");
			die(-1);
		} else {
			sprintf(linebuf, "INPUT: system temperature is %.3f K\n", system->temperature);
			output(linebuf);
		}
	} else if ( system->ensemble == ENSEMBLE_NVE ) {
		sprintf(linebuf, "INPUT: NVE energy is %.3f K\n", system->total_energy);
		output(linebuf);
	}

	if(system->free_volume > 0.0) {
		sprintf(linebuf, "INPUT: system free_volume is %.3f A^3\n", system->free_volume);
		output(linebuf);
	}

	if((system->ensemble == ENSEMBLE_NPT)) {
		if (system->pressure <= 0.0) {
			error("INPUT: invalid pressure set for NPT\n");
			die(-1);
		} else {
			sprintf(linebuf, "INPUT: reservoir pressure is %.3f atm\n", system->pressure);
//		sprintf(linebuf, "INPUT: fugacity is set to %.3f\n", system->fugacity); //???? shouldn't be set in NPT, right?
			output(linebuf);
		}
		if ( system->ewald_alpha != EWALD_ALPHA ) {
			error("INPUT: Ewald alpha cannot be manually set for NPT ensemble.\n");
			die(-1);
		}
	}

	if(system->ensemble == ENSEMBLE_UVT) {
		if(system->user_fugacities) {
			sprintf(linebuf, "INPUT: user defined fugacities are in use.\n");
			output(linebuf);
			for ( i=0; i<system->fugacitiesCount; i++ ) {
				sprintf(linebuf, "INPUT: fugacity[%d] is set to %.3f atm\n", i, system->fugacities[i]);
				output(linebuf);
			}
			if ( system->pressure != 0.0 ) {
				sprintf(linebuf, "INPUT: user defined fugacities are not compatible with pressure specification.\n");
				output(linebuf);
				die(-1);
			}
		}
		else if (system->pressure <= 0.0) {
			error("INPUT: invalid pressure set for GCMC\n");
			die(-1);
		} else {
			if(system->ensemble == ENSEMBLE_UVT) {
				sprintf(linebuf, "INPUT: reservoir pressure is %.3f atm\n", system->pressure);
	//		sprintf(linebuf, "INPUT: fugacity is set to %.3f\n", system->fugacity); //not yet set?
				output(linebuf);
			}

			if(system->h2_fugacity) {

				if ( system->fugacities != NULL ) {
					error("INPUT: h2_fugacity called, but fugacities are already set.\n");
					die(-1);
				}
				system->fugacities = malloc(sizeof(double));
				memnullcheck(system->fugacities,sizeof(double),__LINE__-1, __FILE__);

				system->fugacities[0] = h2_fugacity(system->temperature, system->pressure);
				if(system->h2_fugacity == 0.0) {
					error("INPUT: error in H2 fugacity assignment\n");
					die(-1);
				}
				sprintf(linebuf, "INPUT: H2 fugacity = %.3f atm\n", system->fugacities[0]);
				output(linebuf);
			}

			if(system->co2_fugacity) {

				if ( system->fugacities != NULL ) {
					error("INPUT: co2_fugacity called, but fugacities are already set.\n");
					die(-1);
				}
				system->fugacities = malloc(sizeof(double));
				memnullcheck(system->fugacities,sizeof(double),__LINE__-1, __FILE__);

				system->fugacities[0] = co2_fugacity(system->temperature, system->pressure);
				if(system->co2_fugacity == 0.0) {
					error("INPUT: error in CO2 fugacity assignment\n");
					die(-1);
				}

				sprintf(linebuf, "INPUT: CO2 fugacity = %.3f atm\n", system->fugacities[0]);
				output(linebuf);
			}

			if(system->ch4_fugacity) {

				if ( system->fugacities != NULL ) {
					error("INPUT: ch4_fugacity called, but fugacities are already set.\n");
					die(-1);
				}
				system->fugacities = malloc(sizeof(double));
				memnullcheck(system->fugacities,sizeof(double),__LINE__-1, __FILE__);

				system->fugacities[0] = ch4_fugacity(system->temperature, system->pressure);
				if(system->ch4_fugacity == 0.0) {
					error("INPUT: error in CH4 fugacity assignment\n");
					die(-1);
				}

				sprintf(linebuf, "INPUT: CH4 fugacity = %.3f atm\n", system->fugacities[0]);
				output(linebuf);
			}

			if(system->n2_fugacity) {

				if ( system->fugacities != NULL ) {
					error("INPUT: n2_fugacity called, but fugacities are already set.\n");
					die(-1);
				}
				system->fugacities = malloc(sizeof(double));
				memnullcheck(system->fugacities,sizeof(double),__LINE__-1, __FILE__);

				system->fugacities[0] = n2_fugacity(system->temperature, system->pressure);
				if(system->n2_fugacity == 0.0) {
					error("INPUT: error in N2 fugacity assignment\n");
					die(-1);
				}

				sprintf(linebuf, "INPUT: N2 fugacity = %.3f atm\n", system->fugacities[0]);
				output(linebuf);
			}
		} //calculated fugacities
	} //ensemble UVT

	sprintf(linebuf, "INPUT: insert probability is %.3f\n", system->insert_probability);
	output(linebuf);
	sprintf(linebuf, "INPUT: move probability is %.3f\n", system->move_probability);
	output(linebuf);
	sprintf(linebuf, "INPUT: gwp probability is %.3f\n", system->gwp_probability);
	output(linebuf);
	sprintf(linebuf, "INPUT: rotation probability is %.3f\n", system->rot_probability);
	output(linebuf);
	sprintf(linebuf, "INPUT: spinflip probability is %.3f\n", system->spinflip_probability);
	output(linebuf);

	if ( system->ensemble == ENSEMBLE_NPT ) {
		if ( system->volume_probability == 0.0 )
			sprintf(linebuf, "INPUT: volume change probability is 1/N_molecules.\n");
		else
			sprintf(linebuf, "INPUT: volume change probability is %.3f\n", system->volume_probability);
		output(linebuf);

		sprintf(linebuf, "INPUT: volume change factor is %lf.\n", system->volume_change_factor);
		output(linebuf);
	}

	/* autoreject insertions closer than some scaling factor of sigma */
	if(system->cavity_autoreject) {
		output("INPUT: cavity autorejection activated\n");
		if((system->cavity_autoreject_scale <= 0.0) || (system->cavity_autoreject_scale > 1.0))
			error("INPUT: cavity_autoreject_scale either not set or out of range\n");
	}

	if(system->cavity_autoreject_absolute) { 
		output("INPUT: cavity autoreject absolute activated\n");
		if((system->cavity_autoreject_scale <= 0.0) || (system->cavity_autoreject_scale > 1.0))
			error("INPUT: cavity_autoreject_scale either not set or out of range\n");
	}
		

	if(system->cavity_bias) {

		if((system->cavity_grid_size <= 0) || (system->cavity_radius <= 0.0)) {
			error("INPUT: invalid cavity grid or radius specified\n");
		} else {
			output("INPUT: cavity-biased umbrella sampling activated\n");
			sprintf(linebuf, "INPUT: cavity grid size is %dx%dx%d points with a sphere radius of %.3f A\n", 
				system->cavity_grid_size, system->cavity_grid_size, system->cavity_grid_size, system->cavity_radius);
			output(linebuf);
		}
	}

	if(system->wpi) {
		output("INPUT: Widom Particle Insertion is enabled\n");
	}

	return;
}


void hist_options ( system_t * system ) {
	char linebuf[MAXLINE];

	output("INPUT: histogram calculation will be performed\n");
	if(!system->hist_resolution){
		output("INPUT: no histogram resolution set but histogram calculation requested\n");
		output("INPUT: setting hist_resolution to default value of 0.7A\n");
		system->hist_resolution=0.7;
	}
	else if(system->hist_resolution<0.01 || system->hist_resolution>5.0){
		output("INPUT: histogram resolution out of bounds\n");
		output("INPUT: setting hist_resolution to default value of 0.7A\n");
		system->hist_resolution=0.7;
	}
	else if(!system->histogram_output){
		output("INPUT: no histogram outputfile selected, defaulting to histogram.dat\n");
		system->histogram_output=calloc(MAXLINE,sizeof(char));
		memnullcheck(system->histogram_output,MAXLINE*sizeof(char),__LINE__-1, __FILE__);
		sprintf(system->histogram_output,"histogram.dat");
	}
	else{
		sprintf(linebuf,"INPUT: histogram resolution set to %.3f A\n",system->hist_resolution);
		output(linebuf);
	}

	if(system->max_bondlength < .5){
		output("INPUT: max_bondlength either not set or out of bounds\n");
		output("INPUT: setting max_bondlength to default value of 1.8A\n");
		system->max_bondlength=1.8;
	}	

	if(!system->frozen_output){
		output("INPUT: no frozen_output set! setting frozen coordinate output file to frozen.dx\n");
		system->frozen_output = calloc(MAXLINE, sizeof(char));
		memnullcheck(system->frozen_output,MAXLINE*sizeof(char),__LINE__-1, __FILE__);
		sprintf(system->frozen_output,"frozen.dx");
	} else {
		sprintf(linebuf, "INPUT: will be writing frozen coordinates to %s\n", system->frozen_output);
		output(linebuf);
	}

	return;
}


void io_files_options(system_t * system) {
	char linebuf[MAXLINE];

	if(!system->pqr_input) {
		system->pqr_input = calloc(MAXLINE,sizeof(char));
		memnullcheck(system->pqr_input,MAXLINE*sizeof(char),__LINE__-1, __FILE__);
		strcpy(system->pqr_input,system->job_name);
		strcat(system->pqr_input,".initial.pqr");
		sprintf(linebuf,"INPUT: input PQR file not specified...will try to read coordinates from ./%s\n", system->pqr_input);
		output(linebuf);
	} else {
		sprintf(linebuf, "INPUT: initial molecular coordinates are in ./%s\n", system->pqr_input);
		output(linebuf);
	}

	if(!system->pqr_output) {	// (CRC)
		system->pqr_output = calloc(MAXLINE,sizeof(char));
		memnullcheck(system->pqr_output,MAXLINE*sizeof(char),__LINE__-1, __FILE__);
		strcpy(system->pqr_output,system->job_name);
		strcat(system->pqr_output,".final.pqr");
		sprintf(linebuf, "INPUT: will be writing final configuration to ./%s\n", system->pqr_output);
		output(linebuf);
	} else if(!strcasecmp(system->pqr_output, "off")) {	// Optionally turn off final configuration output
		error("INPUT: **Warning: PQR final configuration file unspecified; writing to /dev/null\n");
		sprintf(system->pqr_output,"/dev/null");
	} else {
		sprintf(linebuf, "INPUT: will be writing final configuration to ./%s\n", system->pqr_output);
		output(linebuf);
	}

	if(!system->pqr_restart) {	// (CRC)
		system->pqr_restart = calloc(MAXLINE,sizeof(char));
		memnullcheck(system->pqr_restart,MAXLINE*sizeof(char),__LINE__-1, __FILE__);
		strcpy(system->pqr_restart,system->job_name);
		strcat(system->pqr_restart,".restart.pqr");
		sprintf(linebuf, "INPUT: will be writing restart configuration to ./%s\n", system->pqr_restart);
		output(linebuf);
	} else if(!strcasecmp(system->pqr_restart, "off")) {	// Optionally turn off restart configuration output
		error("INPUT: **Warning: PQR restart file unspecified; writing to /dev/null\n");
		sprintf(system->pqr_restart,"/dev/null");
	} else {
		sprintf(linebuf, "INPUT: will be writing restart configuration to ./%s\n", system->pqr_restart);
		output(linebuf);
	}

	/* NEW: Energy output will default to on if not specified */
	if(!system->energy_output) {	// (CRC)
		system->energy_output = calloc(MAXLINE,sizeof(char));
		memnullcheck(system->energy_output,MAXLINE*sizeof(char),__LINE__-1, __FILE__);
		strcpy(system->energy_output,system->job_name);
		strcat(system->energy_output,".energy.dat");
		sprintf(linebuf, "INPUT: will be writing energy output to ./%s\n", system->energy_output);
		output(linebuf);
	} else if(!strcasecmp(system->energy_output, "off")) {	// Optionally turn off energy printing
		error("INPUT: energy file unspecified; writing to /dev/null\n");
		sprintf(system->energy_output,"/dev/null");
	} else {
		sprintf(linebuf, "INPUT: will be writing energy output to ./%s\n", system->energy_output);
		output(linebuf);
	}

	/* NEW: Trajectory file will default to on if not specified */
	if(!system->traj_output) {	// (CRC)
		system->traj_output = calloc(MAXLINE,sizeof(char));
		memnullcheck(system->traj_output,MAXLINE*sizeof(char),__LINE__-1, __FILE__);
		strcpy(system->traj_output,system->job_name);
		strcat(system->traj_output,".traj.pqr");
		sprintf(linebuf, "INPUT: will be writing trajectory to ./%s\n", system->traj_output);
		output(linebuf);
	} else if(!strcasecmp(system->traj_output, "off")) {	// Optionally turn off trajectory printing
		error("INPUT: trajectory file will not be written\n");
		sprintf(system->traj_output,"/dev/null"); //won't actually write (we check to see where we're writing before actually doing it for traj.pqr)
	} else {
		sprintf(linebuf, "INPUT: will be writing trajectory to ./%s\n", system->traj_output);
		output(linebuf);
	}

	if(system->insert_input) {
		sprintf( linebuf, "INPUT: inserted molecules will be selected from ./%s\n", system->insert_input );
		output( linebuf );
	} 

	if(system->polarization && !system->dipole_output) {	// (CRC)
		system->dipole_output = calloc(MAXLINE,sizeof(char));
		memnullcheck(system->dipole_output,MAXLINE*sizeof(char),__LINE__-1, __FILE__);
		strcpy(system->dipole_output,system->job_name);
		strcat(system->dipole_output,".dipole.dat");
		sprintf(linebuf, "INPUT: dipole field will be written to ./%s\n", system->dipole_output);
		output(linebuf);
	} else if( (system->polarization) && (!strcasecmp(system->dipole_output, "off")) ) {
		error("INPUT: dipole file unspecified; writing to /dev/null\n");
		sprintf(system->dipole_output,"/dev/null");
	} else if(system->polarization) {
		sprintf(linebuf, "INPUT: dipole field will be written to ./%s\n", system->dipole_output);
		output(linebuf);
	}

	if(system->polarization && !system->field_output) {	// (CRC)
		system->field_output = calloc(MAXLINE,sizeof(char));
		memnullcheck(system->field_output,MAXLINE*sizeof(char),__LINE__-1, __FILE__);
		strcpy(system->field_output,system->job_name);
		strcat(system->field_output,".field.dat");
		sprintf(linebuf, "INPUT: field field will be written to ./%s\n", system->field_output);
		output(linebuf);
	} else if( (system->polarization) && (!strcasecmp(system->field_output, "off")) ) {
		error("INPUT: field file unspecified; writing to /dev/null\n");
		sprintf(system->field_output,"/dev/null");
	} else if(system->polarization) {
		sprintf(linebuf, "INPUT: field field will be written to ./%s\n", system->field_output);
		output(linebuf);
	}

	return;
}



int check_system(system_t *system) {

	char linebuf[MAXLINE];
	
	check_ensemble(system,system->ensemble);

	if(system->ensemble == ENSEMBLE_SURF_FIT) ensemble_surf_fit_options(system);
	else if(system->ensemble == ENSEMBLE_SURF) ensemble_surf_options(system);
	else if(system->ensemble == ENSEMBLE_TE) ensemble_te_options(system);
	else if(system->ensemble == ENSEMBLE_REPLAY) ensemble_replay_options(system);
	else mc_options(system);
	if(system->spectre) spectre_options(system);
	if(system->rd_only) output("INPUT: calculating repulsion/dispersion only\n");
	if(system->wolf) output("INPUT: ES Wolf summation active\n");
	if(system->rd_lrc) output("INPUT: rd long-range corrections are ON\n");
		else output("INPUT: rd long-range corrections are OFF\n");
	if(system->sg) output("INPUT: Molecular potential is Silvera-Goldman\n");
	if(system->waldmanhagler) output("INPUT: Using Waldman-Hagler mixing rules for LJ-interactions.\n");
	if(system->dreiding) output("INPUT: Molecular potential is DREIDING\n");
	if(system->feynman_hibbs) feynman_hibbs_options(system);
	if(system->simulated_annealing) simulated_annealing_options(system);
	if(system->calc_hist) hist_options(system);
	if(system->polarization) polarization_options(system);
#ifdef QM_ROTATION
	if(system->quantum_rotation) qrot_options(system);
#endif 
#ifdef XXX
	if(system->quantum_vibration) output("INPUT: Quantum vibrational eigenspectrum calculation enabled\n");
#endif /* XXX */

	// Require a job name (CRC)
	if(!system->job_name) {
		error("INPUT: must specify a job name\n");
		return(-1);
	} else {
		sprintf(linebuf, "INPUT: Job Name: %s\n", system->job_name);
		output(linebuf);
		io_files_options(system);
	}

	//miscellaneous options

	if(system->gwp) {
		output("INPUT: Gaussian wavepacket code active\n");
		if(system->gwp_probability == 0.) {
			output("INPUT: GWP move scaling not input - setting equal to move_probability\n");
			system->gwp_probability = system->move_probability;
		}
	}

	if ( system->scale_charge != 1.0 ) {
		sprintf(linebuf, "INPUT: frozen atom charges scaled by %.2f\n", system->scale_charge);
		output(linebuf);
	}
	
	if ( system->scale_rd != 1.0 ) {
		sprintf(linebuf, "INPUT: frozen atom rd scaled by %.2f\n", system->scale_rd);
		output(linebuf);
	}

	if(system->cuda) {
#ifndef CUDA
		error("INPUT: cuda keyword enabled, but not compiled into this code\n");
		return(-1);
#else
		output("INPUT: CUDA GPU acceleration activated\n");
#endif /* CUDA */
	}

	if(system->rd_anharmonic) {
		if(!system->rd_only) {
			error("INPUT: rd_anharmonic being set requires rd_only\n");
			return(-1);
		} else {
			sprintf(linebuf, "INPUT: rd_anharmonic_k = %.3f K/A^2\n", system->rd_anharmonic_k);
			output(linebuf);
			sprintf(linebuf, "INPUT: rd_anharmonic_g = %.3f K/A^4\n", system->rd_anharmonic_g);
			output(linebuf);
		}
	}

	return(0);
}
