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
    
    // Задаем границы интегрирования.
    double result = (function(0) + function(num_x * h)) / 2;
    
    /* 
        Статическое распределение итераций с шагом в 10000.
        Правильное суммирование всех параллельных вычислений
        с помощью reduction.
    */
    #pragma omp parallel for schedule(static, 10000) reduction(+: result)
    for (int i = 1; i < num_x; i++) {
        result += function(i * h);
    }
    result *= h;

    // Выводим значение интеграла.
    printf("%lf\n", result);
    return 0;
}