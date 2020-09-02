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
    
    // Инициализируем MPI-окружение.
    MPI_Init(&argc, &argv);
    
    // Определяем размер коммуникатора и ранг процесса.
    int size, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); 
    
    /*
        Массив border_ponts содержит все граничные точки между процессами.
        То есть border_points[i] есть номер точки между i и (i+1) процессами,
        которую считает процесс i.
    */
    int *border_points = (int*) malloc(sizeof(int) * (size - 1));
    
    /* Заполняем массив с шагом в (num_x / Size). */
    border_points[0] = num_x / size;
    for (int i = 1; i < size - 1; i++) {
        border_points[i] = border_points[i - 1] + (num_x / size);
    }
    
    for(int i = 0; i < num_time; i++) {
        /* Обмениваемся левым и правым граничными узлами. */
        if (rank != 0)
            MPI_Send(arr_n + border_points[rank - 1], 1, MPI_DOUBLE, rank - 1, 0, MPI_COMM_WORLD);
        if (rank != size - 1) 
            MPI_Send(arr_n + border_points[rank] - 1, 1, MPI_DOUBLE, rank + 1, 0, MPI_COMM_WORLD);
        if (rank != 0)
            MPI_Recv(arr_n + border_points[rank - 1] - 1, 1, MPI_DOUBLE, rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        if (rank != size - 1)
            MPI_Recv(arr_n + border_points[rank], 1, MPI_DOUBLE, rank + 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            
        /* Задаем левую и правую границы, которые обсчитывает этот процесс. */
        int left_border_iter = 1;
        if (rank > 0) left_border_iter = border_points[rank - 1];
        int right_border_iter = num_x;
        if (rank < size - 1) right_border_iter = border_points[rank];
        
        /* Высчитываем следующий слой. */
        for (int j = left_border_iter; j < right_border_iter; j++) {
            arr_n_plus[j] = arr_n[j] + tau / h / h * (arr_n[j-1] - 2.0 * arr_n[j] + arr_n[j+1]);
        }
        
        /* Если процессы обсчитывают граничные точки, то определяем их из граничных условий. */
        if (rank == 0) 
            arr_n_plus[0] = left_border(tau * (i + 1));
        if (rank == size - 1) 
            arr_n_plus[num_x] = right_border(tau * (i + 1));
        
        /* Обмениваем указатели на два последовательных массива. */
        double *save = arr_n;
        arr_n = arr_n_plus;
        arr_n_plus = save;
    }
    
    // Отправляем данные на нулевой процесс.
    if (rank > 0) {
        int num_to_send = (rank == size - 1)? num_x - border_points[rank - 1] + 1: 
            border_points[rank] - border_points[rank - 1];
        MPI_Send(arr_n + border_points[rank - 1], num_to_send, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
    }
    
    // Принимаем данные на нулевом процессе.
    for (int i = 1; i < size && rank == 0; i++) {
        int num_to_recv = (i == size - 1)? num_x - border_points[i - 1] + 1 : border_points[i] - border_points[i - 1];
        MPI_Recv(arr_n + border_points[i - 1], num_to_recv, MPI_DOUBLE, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);       
    }
    
    // Выводим результат.
    for (int i = 0; i <= num_x && rank == 0; i++) {
        printf("%.2lf %lf\n", i * h, arr_n[i]);
    }
    
    // Освобождаем память.
    free(arr_n);
    free(arr_n_plus);
    free(border_points);
    
    // Прощаемся с MPI.
    MPI_Finalize();
    
    return 0;
}