#include<stdlib.h> /* atoi, atof */
#include<mpi.h>

#define LENGTH 10.0

/*
    Функции возвращают начальное 
    распределение, температуры с левой 
    и правой границ соответственно.
*/
double start_function  (double x) { return 0.0; }
double left_border  (double time) { return 1.0; }
double right_border (double time) { return 2.0; }

int main (int argc, char **argv) {
    
    /*
        Если количество переданных параметров не то,
        которое нам нужно, то сорян: программа работать
        не будет.
    */
    if (argc != 3) {
        printf ("INCORRECT INPUT :(\n");
        return 1;
    }
   
    /*
        Считываем время, при котором мы должны найти функцию, 
        и количество точек дискретизации нашего стержня.
    */
    double time = atof(argv[1]);
    int num_x = atoi(argv[2]);

    // Задаем шаг по оси X.
    double h = LENGTH / num_x;
    
    // Шаг по времени задаем так, чтобы не развалился алгоритм.
    double tau = 0.3 * h * h;
    
    // Находим число шагов по времени, которое надо сделать.
    int num_time = time / tau;
    
    /*
        Создаем два массива для последовательного вычисления
        значений нашей функции.
    */
    double *arr_n      = (double *) malloc(sizeof(double) * (num_x + 1));
    double *arr_n_plus = (double *) malloc(sizeof(double) * (num_x + 1));
    
    /*
        Определяем начальный массив, не забывая 
        о граничных условиях.
    */
    for (int i = 1; i < num_x; i++) arr_n[i] = start_function(i * h);
    arr_n[0] = left_border(0.0);
    arr_n[num_x] = right_border(0.0);
     
    for(int i = 0; i < num_time; i++) {
        /*
            Высчитываем следующий слой.
        */
        for (int j = 1; j < num_x; j++) {
            arr_n_plus[j] = arr_n[j] + tau / h / h * (arr_n[j-1] - 2.0 * arr_n[j] + arr_n[j+1]);
        }
        arr_n_plus[0] = left_border(tau * (i + 1));
        arr_n_plus[num_x] = right_border(tau * (i + 1));
        /*
            Обмениваем указатели на два последовательных массива.
        */
        double *save = arr_n;
        arr_n = arr_n_plus;
        arr_n_plus = arr_n;
    }
    
    // Выводим результат.
    for (int i = 0; i <= num_x; i++) {
        printf("%lf %lf\n", i * h, arr_n[i]);
    }
    
    // Освобождаем память.
    free(arr_n);
    free(arr_n_plus);
    
    return 0;
}