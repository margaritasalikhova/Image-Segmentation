//
//  main.cpp
//  opencvproj
//
//  Created by Админ on 07/08/2020.
//  Copyright © 2020 Админ. All rights reserved.
//

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <vector>
using namespace std;
using namespace cv;
int main()
{
    // Подготовительная работа
    
    
    setlocale(LC_ALL, "Russian"); // Подключаю русский язык в консоли
    Mat img =imread("1.jpg", IMREAD_COLOR);// Открываю картинку в папке "debug" с исполняемым файлом
    
    if(!img.data) // Если не удалось открыть картинку - нет такого файла
    {
        cout << "НЕ УДАЛОСЬ ОТКРЫТЬ КАРТИНКУ!"<< endl;
        return -1;// Возвращаю -1 как ошибочное завершение программы
    }
    
    cout << "ВЫВОД ИСХОДНОЙ КАРТИНКИ..." << endl; //Сообщение о выводе оригинльной картинки
    
    
    // Основной код
    
    
    imshow ("Original image", img); // Функция вывода картинки в виде окна с заголовком
    waitKey(); // Ждем нажатия любой клавиши для показа следующей картинки
    Mat gry;// То же изображение в чб
    
    cvtColor(img,gry, COLOR_RGB2GRAY);// Функция смены цветовой схемы с rgb в gray (чб) - запись результата в gry
    cout << "ВЫВОД ЧБ ИЗОБРАЖЕНИЯ"<< endl; // Отладочное сообщение о действиях программы
    
    imshow("Grey image", gry);// Вывожу чб изображение
    waitKey();
    Mat trsh; // Изображение, прошедшее пороговое преобразование
    //adaptiveThreshold(gry, trsh, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY, 11, 2);//адаптивное пороговое преобразование с применением фильтра Гаусса, на некоторых изображениях оно работает лучше
 
    threshold(gry, trsh, 0, 255, THRESH_BINARY+THRESH_OTSU); // Пороговое преобразование - отбор пикселей выше/ниже определенного значения, porog- то самое значение, 255 - максимальное значение, thresh_binary+thresh_otsu - тип преобразования BINARY - при отборе пикселей, если значение больше порога - приравниваем его к максимальному, иначе к нулю OTSU - автоматически выбираем порог (можем поставить 0, а не подбирать числа в пределах 40-60 в зависимости от картинки)
    
    
    
    //это нужно, чтобы получить картинку только из исключительно черных и белых цветов (без серых градаций). если какой-то пикселт достаточно темный, то мы его делаем полностью черным, есои он не дотягиваеь до порога - делаем его белым, так как такое значение сложно подобрать под каждую картинку для упрощения ставим опцию OTSU. она по сути "измеряет" средние значения в картинке и сама определяет это значение
    
    
    Mat kern = Mat::ones(3,3,CV_8U); // Ядро для преобразования 3 на 3
    morphologyEx(trsh, trsh, MORPH_OPEN, kern);// Морфологическое преобразование для сглаживания некоторых мелких деталей
    
    cout << "ВЫВОД ЧБ ГРАНИЦ С АВТОМАТИЧЕСКИМ ПОРОГОМ" << endl;
    imshow("Threshold image",trsh);//выводим преобразованное чб
    waitKey();
    
    Mat distTrans;// Еще одно преобразованное изображение
    distanceTransform(trsh, distTrans, DIST_L2, 3);// Преобразование в замене значений интенсивности пикселей на дистанцию от фона
    int normalizeCoef = 100;
    cout << "НОРМАЛИЗАЦИЯ С КОЭФФИЦИЕНТОМ " << normalizeCoef << endl;
    normalize(distTrans, distTrans, 0, normalizeCoef, NORM_MINMAX);// Нормализация картинки минимаксным методом (почему-то иногда портит результат, тогда строку нужно закомментировать)
    threshold(distTrans, distTrans, 0.5, 1, THRESH_BINARY);// Еще раз применяем пороговое преобразование
    
    imshow("Distance transformed", distTrans);
    waitKey();
    Mat distTrans8u;
    distTrans.convertTo(distTrans8u, CV_8U);// 8 битное изображение, нужно для дальнейшей обработки
    vector<vector<Point>> contours;// Контуры представляю собой векторы точек
    findContours(distTrans8u, contours,RETR_TREE, CHAIN_APPROX_SIMPLE);// Ищу контуры на изображении, RETR_TREE определяет как мы строим иерархию контуров - извлекаем все, на всякий случай, APPROX_SIMPLE - стандартный метод аппроксимации - способа поиска контуров
    
    cout << "Найдено контуров: " << contours.size() << endl;
    
    Mat marks = Mat::zeros(distTrans.size(), CV_32S); // Маркеры, по которым мы выделим сегменты картинки
    
    for (int i=0; i<contours.size(); i++) // Цикл, обходящий все контуры
        drawContours(marks, contours, i, Scalar(i+1), -1); // Рисуем контуры
    circle(marks, Point(5,5), 3, CV_RGB(255,255,255), -1); // Обрисовываем конутры
    watershed(img, marks);// Используем алгоритм "водораздела"
    vector<Vec3b> colors; // Вектор цветов
    int r,g,b; // Значения каналов красный зеленый синий
    for (int i = 0; i < contours.size(); i++) // Обходим контуры, рандомно присоединяем к контуру цвет
    {
        r = theRNG().uniform(0, 255);
        g = theRNG().uniform(0, 255);
        b = theRNG().uniform(0, 255);//рандомно формируем значение каждого канала (от 0 до макс 255)
        Vec3b color((uchar)b, (uchar)g, (uchar)r);// Создаем цвет
        colors.push_back(color);// Присоединяем цвет в массив цветов
    }
    Mat result = Mat::zeros(marks.size(), CV_8UC3);// Создаем результирующее изображение
    int ind;// Индекс для обработки маркеров
    for (int i = 0; i < marks.rows; i++)// Обходим матрицу маркеров по строкам
        for (int j = 0; j < marks.cols; j++)// И столбцам
        {
            ind = marks.at<int>(i,j);
            if (ind > 0 && ind <= contours.size())// Если индекс положителен и меньше или равен числу контуров
                result.at<Vec3b>(i,j) = colors[ind-1];// Назначаем цвет из таблицы цветов по индексу
            else
                result.at<Vec3b>(i,j) = Vec3b(0,0,0);// Иначе инициализируем значение в векторе нулями
        }
    
    cv::imshow("Result", result);// Вывод результата
    waitKey();// Ждем кнопку
    cout<<"ЗАВЕРШЕНИЕ ПРОГРАММЫ"<<endl;// Завершаем программу
    return 0; // Возвращаем нолик -- все хорошо :)
}
