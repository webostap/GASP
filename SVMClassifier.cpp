#include "SVMClassifier.h"

namespace ps {

	SVMClassifier::SVMClassifier()
	{

	}

	SVMClassifier::~SVMClassifier()
	{
	}



	void SVMClassifier::train(std::vector<Particle> particles_list, const char* input_params)
	{
		// guard
		if (particles_list.empty()) return;

		svm_parameter param;
		int i, j;

		// default values
		param.svm_type = C_SVC;
		param.kernel_type = RBF;
		param.degree = 3;
		param.gamma = 0;
		param.coef0 = 0;
		param.nu = 0.5;
		param.cache_size = 100;
		param.C = 1;
		param.eps = 1e-3;
		param.p = 0.1;
		param.shrinking = 1;
		param.probability = 0;
		param.nr_weight = 0;
		param.weight_label = NULL;
		param.weight = NULL;

		// parse options
		const char* p = input_params;

		while (1) {
			while (*p && *p != '-')
				p++;

			if (*p == '\0')
				break;

			p++;
			switch (*p++) {
			case 's':
				param.svm_type = atoi(p);
				break;
			case 't':
				param.kernel_type = atoi(p);
				break;
			case 'd':
				param.degree = atoi(p);
				break;
			case 'g':
				param.gamma = atof(p);
				break;
			case 'r':
				param.coef0 = atof(p);
				break;
			case 'n':
				param.nu = atof(p);
				break;
			case 'm':
				param.cache_size = atof(p);
				break;
			case 'c':
				param.C = atof(p);
				break;
			case 'e':
				param.eps = atof(p);
				break;
			case 'p':
				param.p = atof(p);
				break;
			case 'h':
				param.shrinking = atoi(p);
				break;
			case 'b':
				param.probability = atoi(p);
				break;
			case 'w':
				++param.nr_weight;
				param.weight_label = (int*)realloc(param.weight_label, sizeof(int) * param.nr_weight);
				param.weight = (double*)realloc(param.weight, sizeof(double) * param.nr_weight);
				param.weight_label[param.nr_weight - 1] = atoi(p);
				while (*p && !isspace(*p)) ++p;
				param.weight[param.nr_weight - 1] = atof(p);
				break;
			}
		}

		// build problem
		svm_problem prob;

		prob.l = particles_list.size();
		prob.y = new double[prob.l];

		if (param.kernel_type == PRECOMPUTED)
		{
		}
		else if (param.svm_type == EPSILON_SVR || param.svm_type == NU_SVR)
		{
			if (param.gamma == 0) param.gamma = 1;
			svm_node* x_space = new svm_node[2 * prob.l];
			prob.x = new svm_node * [prob.l];



			for (int i = 0; i < particles_list.size(); i++)
			{
				x_space[2 * i].index = 1;
				x_space[2 * i].value = particles_list[i].x;
				x_space[2 * i + 1].index = -1;
				prob.x[i] = &x_space[2 * i];
				prob.y[i] = particles_list[i].z;
			}

			// build model & classify
			svm_model* model = svm_train(&prob, &param);
			svm_node x[2];
			x[0].index = 1;
			x[1].index = -1;
			int* j = new int[XLEN];

			for (i = 0; i < XLEN; i++)
			{
				x[0].value = (double)i / XLEN;
				j[i] = (int)(YLEN * svm_predict(model, x));
			}

			buffer_painter.setPen(colors[0]);
			buffer_painter.drawLine(0, 0, 0, YLEN - 1);

			int p = (int)(param.p * YLEN);
			for (i = 1; i < XLEN; i++)
			{
				buffer_painter.setPen(colors[0]);
				buffer_painter.drawLine(i, 0, i, YLEN - 1);

				buffer_painter.setPen(colors[5]);
				buffer_painter.drawLine(i - 1, j[i - 1], i, j[i]);

				if (param.svm_type == EPSILON_SVR)
				{
					buffer_painter.setPen(colors[2]);
					buffer_painter.drawLine(i - 1, j[i - 1] + p, i, j[i] + p);

					buffer_painter.setPen(colors[2]);
					buffer_painter.drawLine(i - 1, j[i - 1] - p, i, j[i] - p);
				}
			}

			svm_free_and_destroy_model(&model);
			delete[] j;
			delete[] x_space;
			delete[] prob.x;
			delete[] prob.y;
		}
		else
		{
			if (param.gamma == 0) param.gamma = 0.5;
			svm_node* x_space = new svm_node[3 * prob.l];
			prob.x = new svm_node * [prob.l];

			for (int i = 0; i < particles_list.size(); i++)
			{
				x_space[3 * i].index = 1;
				x_space[3 * i].value = particles_list[i].x;
				x_space[3 * i + 1].index = 2;
				x_space[3 * i + 1].value = particles_list[i].z;
				x_space[3 * i + 2].index = -1;
				prob.x[i] = &x_space[3 * i];
				prob.y[i] = static_cast<int>(particles_list[i].state);
			}

			// build model & classify
			svm_model* model = svm_train(&prob, &param);
			svm_node x[3];
			x[0].index = 1;
			x[1].index = 2;
			x[2].index = -1;

			for (i = 0; i < XLEN; i++)
				for (j = 0; j < YLEN; j++) {
					x[0].value = (double)i / XLEN;
					x[1].value = (double)j / YLEN;
					double d = svm_predict(model, x);
					if (param.svm_type == ONE_CLASS && d < 0) d = 2;
					buffer_painter.setPen(colors[(int)d]);
					buffer_painter.drawPoint(i, j);
				}

			svm_free_and_destroy_model(&model);
			delete[] x_space;
			delete[] prob.x;
			delete[] prob.y;
		}
		free(param.weight_label);
		free(param.weight);
	}

}