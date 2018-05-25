
#include <iostream>
#include <ctime>
#include "mpi.h"


#define VMAX 15				//最大速度
#define P 0.25				//减速概率

int clocknum = 2000;		//周期数
int carnum = 100000;		//车的数量
int positionnum;			//道路的估计长度
int *position;				// i车当前的位置
int *oldposition;			// i车之前的位置
int *carnum_in_position;    // 道路i位置处车的数量
int *velocity;				// i车的速度

int myid, numprocs;
int countn = 0;
double starttime, endtime;
double t1, t2;

void updateVelocityPosition(int index){
	int distance = 1;
	int v = velocity[index];
	while (distance <= VMAX)
	{
		if (carnum_in_position[position[index] + distance] > 0)
			break;
		distance++;
	}
	// 前面没车++
	if (distance > v && velocity[index] < VMAX)
		velocity[index]++;
	// 前面有车,d <= v, v = d-1;
	else if (distance <= v)
		velocity[index] = distance - 1;
	// 按概率--
	if (rand() / (double)RAND_MAX <= P)
		velocity[index]--;
	// 保证>=0
	if (velocity[index] < 0)
		velocity[index] = 0;
	position[index] += velocity[index];
}

// 处理carnum_in_position,更新oldposition
void updateCarnum_in_position()
{
	for (int i = 0; i < carnum; i++) {
		carnum_in_position[oldposition[i]] --;
		carnum_in_position[position[i]] ++;
		oldposition[i] = position[i];
	}
}

// 输出carnum_in_position
void printCarnum_in_position(int cycle) {
	std::cout << "cycle: " << cycle << std::endl;
	for (int i = 0; i < positionnum; i++)
		if (carnum_in_position[i] > 0)
			std::cout << "carnum_in_position[" << i << "]: " << carnum_in_position[i] << std::endl;
}

void printposition() {
	for (int i = 0; i < carnum; i++){
		std::cout << i << ":" << position[i] << " " << velocity[i] << std::endl;
	}
}


int main(int argc, char **argv)
{
	positionnum = clocknum * VMAX;
	position = new int[carnum];
	oldposition = new int[carnum];
	carnum_in_position = new int[positionnum];
	velocity = new int[carnum];
	srand((unsigned)time(NULL));

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &myid);
	MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
	MPI_Bcast(&carnum, 1, MPI_INT, 0, MPI_COMM_WORLD);

	for (int i = (carnum + numprocs - 1) / numprocs * myid; 
		i < (carnum + numprocs - 1) / numprocs * (myid + 1) && i < carnum; i++){
		position[i] = 0;
		oldposition[i] = 0;
		velocity[i] = 0;
	}
	for (int i = (positionnum + numprocs - 1) / numprocs * myid; 
		i < (positionnum + numprocs - 1) / numprocs * (myid + 1) && i < positionnum; i++)
		carnum_in_position[i] = 0;
	carnum_in_position[0] = carnum;
	for (int i = 0; i < numprocs; i++) {
		MPI_Bcast(&position[(carnum + numprocs - 1) / numprocs * i],
			(carnum + numprocs - 1) / numprocs, MPI_INT, i, MPI_COMM_WORLD);
		MPI_Bcast(&oldposition[(carnum + numprocs - 1) / numprocs * i], 
			(carnum + numprocs - 1) / numprocs, MPI_INT, i, MPI_COMM_WORLD);
		MPI_Bcast(&velocity[(carnum + numprocs - 1) / numprocs * i], 
			(carnum + numprocs - 1) / numprocs, MPI_INT, i, MPI_COMM_WORLD);
		MPI_Bcast(&carnum_in_position[(positionnum + numprocs - 1) / numprocs * i], 
			(positionnum + numprocs - 1) / numprocs, MPI_INT, i, MPI_COMM_WORLD);
	}

	if (myid == 0)
		starttime = MPI_Wtime();

	for (int j = 0; j < clocknum; j++){
		for (int i = (carnum + numprocs - 1) / numprocs * myid;
			i < (carnum + numprocs - 1) / numprocs * (myid + 1) && i < carnum; i++) {
			updateVelocityPosition(i);
		}

		for (int i = 0; i < numprocs; i++) {
			MPI_Bcast(&velocity[(carnum + numprocs - 1) / numprocs * i],
				(carnum + numprocs - 1) / numprocs, MPI_INT, i, MPI_COMM_WORLD);
			MPI_Bcast(&position[(carnum + numprocs - 1) / numprocs * i],
				(carnum + numprocs - 1) / numprocs, MPI_INT, i, MPI_COMM_WORLD);
		}

		updateCarnum_in_position();
	}

	if (myid == 0){
		printCarnum_in_position(clocknum);
		printposition();
		endtime = MPI_Wtime();
		t1 = endtime - starttime;
		std::cout << "并行时间 = " << t1 << std::endl;
	}
/*
	if (myid == 0) {
		memset(position, 0, carnum);
		memset(oldposition, 0, carnum);
		memset(carnum_in_position, 0, positionnum);
		memset(velocity, 0, carnum);
		starttime = MPI_Wtime();
		for (int j = 0; j < clocknum; j++) {
			for (int i = 0; i < carnum; i++) {
				updateVelocityPosition(i);
			}
			updateCarnum_in_position();
		}
		endtime = MPI_Wtime();
		t2 = endtime - starttime;
		std::cout << "串行时间 = " << t2 << std::endl;
		std::cout << "加速比 = " << t2/t1 << std::endl;
	}
*/
	MPI_Finalize();
	return 0;
}


