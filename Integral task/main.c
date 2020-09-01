#include <math.h>      // Для функции корня.
#include <mpi.h>       // Отсюда берем функции для распараллеливания.
#include <stdlib.h>    /* atoi */

/*
    Задаем левую границу (LEFT) и правую границу (RIGHT).
    Если нам понадобится поменять отрезок интегрирования,
    то можем просто изменить эти константы.
    
    Кроме того, дефайним функцию, которую интегрируем.
*/
#define LEFT 0.0
#define RIGHT 2.0
#define FUNCTION(x) sqrt(4 - x*x)

double function(double x) {
    // Если вылезем за границы, то просто получим ноль.
    if (x > RIGHT || x < LEFT) return 0; 
    else return FUNCTION(x);
}

// Реализуем подсчет интеграла через метод трапеций
double trapezoidal_integral(double left, double right, double h) {
    /*
        Сначала инициализируем переменную Integral
        полусуммой крайних значений. После этого
        пробегаемся по внутренним точкам и добавляем 
        к интегралу их значения.
    */
    double Integral = (function(right) - function(left)) / 2; 
    do {
        Integral += function(left);
        left += h;
    } while (left < right);
        
    // Чтобы получить значение интеграла, необходимо умножить полученную сумму на h.
    Integral *= h;
    
    return Integral;
}

int main (int argc, char **argv) {
    
    /* 
        Будем считать, что если не задано число разбиений,
        то мы не разбиваем отрезок на части.
    */
    int N = 1;
    
    // Но если параметр задан, то считываем его.
    if (argc >= 2) N = atoi(argv[1]);
    
    // Задаем мелкость разбиения отрезка
    double h = (RIGHT - LEFT) / N;
    
    /*
        Запускаем MPI-окружение и в переменные size и rank
        записываем соответственно количество потоков, ассоциированных
        с коммуникатором, и ранг (номер) текущего процесса.
    */
    MPI_Init(&argc, &argv);
    
    int size, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    
    // Определяем левую границу интегрирования для текущего процесса.
    int left_border = rank * (N / size) * h;
    
    /* 
        Определяем правую границу. Если процесс последний, то правая граница
        будет совпадать с правым 
    */
    int right_border = (rank != size - 1) ? (rank + 1) * (N / size) * h : N * h;

    double result = trapezoidal_integral(left_border, right_border, h);
    
    // Отсылаем результат в нулевой процесс.
    if (rank != 0) MPI_Send (&result, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
    // Иначе
    else {
        for (int i = 1; i < size; i++) {
            double save;
            MPI_Recv(&save, 1, MPI_DOUBLE, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            printf("%d: %.6lf\n", i, save);
            result += save;
        }
        
        // Выводим результат.
        printf("%.6lf\n", result);
    }
    
    // Завершаем программу.
    MPI_Finalize();
    return 0;
}
