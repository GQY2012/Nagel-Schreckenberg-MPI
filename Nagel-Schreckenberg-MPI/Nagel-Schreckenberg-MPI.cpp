
#include <iostream>
#include <ctime>
#include "mpi.h"


#define VMAX 15				//����ٶ�
#define P 0.25				//���ٸ���

int clocknum = 2000;		//������
int carnum = 100000;		//��������
int positionnum;			//��·�Ĺ��Ƴ���
int *position;				// i����ǰ��λ��
int *oldposition;			// i��֮ǰ��λ��
int *carnum_in_position;    // ��·iλ�ô���������
int *velocity;				// i�����ٶ�

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
	// ǰ��û��++
	if (distance > v && velocity[index] < VMAX)
		velocity[index]++;
	// ǰ���г�,d <= v, v = d-1;
	else if (distance <= v)
		velocity[index] = distance - 1;
	// ������--
	if (rand() / (double)RAND_MAX <= P)
		velocity[index]--;
	// ��֤>=0
	if (velocity[index] < 0)
		velocity[index] = 0;
	position[index] += velocity[index];
}

// ����carnum_in_position,����oldposition
void updateCarnum_in_position()
{
	for (int i = 0; i < carnum; i++) {
		carnum_in_position[oldposition[i]] --;
		carnum_in_position[position[i]] ++;
		oldposition[i] = position[i];
	}
}

// ���carnum_in_position
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
		std::cout << "����ʱ�� = " << t1 << std::endl;
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
		std::cout << "����ʱ�� = " << t2 << std::endl;
		std::cout << "���ٱ� = " << t2/t1 << std::endl;
	}
*/
	MPI_Finalize();
	return 0;
}


