#include <math.h>      // Для функции корня.
#include <omp.h>       // Отсюда берем функции для распараллеливания.
#include <stdlib.h>    /* atoi */
#include <stdio.h>     /* printf */

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
    double integral = (function(right) - function(left)) / 2; 
    do {
        integral += function(left);
        left += h;
    } while (left < right);
        
    // Чтобы получить значение интеграла, необходимо умножить полученную сумму на h.
    integral *= h;
    
    return integral;
}

int main(int argc, char **argv) {
       /* 
        Будем считать, что если не задано число разбиений,
        то мы не разбиваем отрезок на части.
    */
    int num_x = 1;
    
    // Но если параметр задан, то считываем его.
    if (argc >= 2) num_x = atoi(argv[1]);
    
    // Задаем мелкость разбиения отрезка.
    double h = (RIGHT - LEFT) / num_x;
    
    // Инициализируем результат.
    double result = 0;
    
    /* Создаем и инициализируем замок */
    omp_lock_t lock;
    omp_init_lock (&lock);
    
    #pragma omp parallel 
    {
        // Узнаем ранг процесса и их количество.
        int rank = omp_get_thread_num();
        int size = omp_get_num_threads();
            
        // Устанавливаем левую и правую границы интегрирования.
        int left_border = rank * (num_x / size);
        int right_border = (rank != size - 1) ? (rank + 1) * (num_x / size) : num_x;
           
        // Находим интеграл.
        double integral = trapezoidal_integral (left_border, right_border, h);
            
        /*
            Добавляем к общей сумме значение полученного
            интеграла. Чтобы синхронизировать суммирования,
            используем критическую секцию.
        */
        omp_set_lock(&lock);
        result += integral;
        omp_unset_lock(&lock);
    }
    
    // Удаляем замок.
    omp_destroy_lock(&lock);

    // Выводим значение интеграла.
    printf("%lf\n", result);
    
    return 0;
}