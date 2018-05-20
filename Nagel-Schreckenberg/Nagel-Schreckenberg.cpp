#include <iostream>
#include <time.h>
#include <Windows.h>

#define vmax 15
#define p 0.25
#define carnum 10000
#define clocknum 2000

typedef struct Car{
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

int main()
{
	Car *car = new Car[carnum];
	Car *newcar = new Car[carnum];
	srand((unsigned)time(NULL));
	int countn = 0;
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
		std::cout << countn++ << std::endl;
	}
	std::cout << "-----------------" << std::endl;
	for (int i = 0;i < carnum;i++) {
		std::cout << (*(car + i)).position << " " << (*(car + i)).v << std::endl;
	}

	system("pause");
	return 0;
}