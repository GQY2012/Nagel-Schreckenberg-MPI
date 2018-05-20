#include <iostream>
#include <time.h>
#include <mpi.h>
#include <Windows.h>

#define vmax 15
#define p 0.25
#define carnum 10000
#define clocknum 2000

typedef struct Car {
	int position = 0;
	int v = 0;
}Car;

int getd(Car *car, int index) {
	int min_d = INT_MAX;
	for (int i = 0;i < carnum;i++) {
		if (i == index)
			continue;
		if ((*(car + index)).position < (*(car + i)).position) {
			int d = (*(car + i)).position - (*(car + index)).position;
			min_d = min_d > d ? d : min_d;
		}
	}
	return min_d;
}

int main(int argc, char *argv[])
{
	int myid, numprocs;
	int n = carnum;
	int countn = 0;
	double starttime, endtime;
	double t1, t2;

	Car *car = new Car[carnum];
	Car *newcar = new Car[carnum];
	srand((unsigned)time(NULL));

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &myid);
	MPI_Comm_size(MPI_COMM_WORLD, &numprocs);

	if (myid == 0) {
		starttime = MPI_Wtime();
	}

	for (int j = 0;j < clocknum;j++) {

		MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
		for (int i = myid;i < n;i += numprocs) {
			int d = getd(car, i);
			if (d != INT_MAX) {//前面有车
				if (d <= (*(car + i)).v) {
					(*(newcar + i)).v = d - 1;
				}
				else {
					(*(newcar + i)).v = (*(car + i)).v + 1;//仍然加速
				}
			}
			else {//前面没车
				if ((*(car + i)).v == vmax) {
					(*(newcar + i)).v = (*(car + i)).v;
				}
				else {
					(*(newcar + i)).v = (*(car + i)).v + 1;
				}
			}
			if (1 == rand() % 4) {
				if ((*(newcar + i)).v > 0)
					(*(newcar + i)).v--;
			}
			(*(newcar + i)).position = (*(car + i)).position + (*(newcar + i)).v;
		}
		MPI_Barrier(MPI_COMM_WORLD);//进程同步
		if (myid == 0) {
			Car *temp = car;
			car = newcar;
			newcar = temp;
			
//			for (int i = 0;i < carnum;i++) {
//			std::cout << (*(car + i)).position << " " << (*(car + i)).v << " " << getd(car, i) << std::endl;
//			}
//			std::cout << "-----------------" << std::endl;
//			std::cout << countn++ << std::endl;
		}
	}
	MPI_Barrier(MPI_COMM_WORLD);//进程同步
	if (myid == 0) {
		endtime = MPI_Wtime();
		t1 = endtime - starttime;
	//	std::cout << "-----------------" << std::endl;
	//	for (int i = 0;i < carnum;i++) {
	//		std::cout << (*(car + i)).position << " " << (*(car + i)).v << std::endl;
	//	}
	}

	if (myid == 0) {
		Car *car = new Car[carnum];
		Car *newcar = new Car[carnum];
		srand((unsigned)time(NULL));
		int countn = 0;
		starttime = MPI_Wtime();
		for (int j = 0;j < clocknum;j++) {
			for (int i = 0;i < carnum;i++) {
				int d = getd(car, i);
				if (d != INT_MAX) {//前面有车
					if (d <= (*(car + i)).v) {
						(*(newcar + i)).v = d - 1;
					}
					else {
						(*(newcar + i)).v = (*(car + i)).v + 1;//仍然加速
					}
				}
				else {//前面没车
					if ((*(car + i)).v == vmax) {
						(*(newcar + i)).v = (*(car + i)).v;
					}
					else {
						(*(newcar + i)).v = (*(car + i)).v + 1;
					}
				}
				if (1 == rand() % 4) {
					if ((*(newcar + i)).v > 0)
						(*(newcar + i)).v--;
				}

				(*(newcar + i)).position = (*(car + i)).position + (*(newcar + i)).v;
			}
			Car *temp = car;
			car = newcar;
			newcar = temp;
			//		for (int i = 0;i < carnum;i++) {
			//			std::cout << (*(car + i)).position << " " << (*(car + i)).v << " " << getd(car, i) << std::endl;
			//		}
			//		std::cout << "-----------------" << std::endl;
			//std::cout << countn++ << std::endl;
		}
		
		endtime = MPI_Wtime();
		t2 = endtime - starttime;

		//std::cout << "-----------------" << std::endl;
		//for (int i = 0;i < carnum;i++) {
		//	std::cout << (*(car + i)).position << " " << (*(car + i)).v << std::endl;
		//}

		printf("并行时间：%fs\n", t1);
		printf("串行时间：%fs\n", t2);
		printf("加速比：%f\n", t2 / t1);
	}

	MPI_Finalize();
	return 0;
}