#include <iostream>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <vector>
#include <iomanip>

using namespace std;

bool button_is_pressed() {
    termios term;
    tcgetattr(0, &term);


    termios term2 = term;
    term2.c_lflag &= ~ICANON;
    tcsetattr(0, TCSANOW, &term2);

    int byteswaiting;
    ioctl(0, FIONREAD, &byteswaiting);

    tcsetattr(0, TCSANOW, &term);

    return byteswaiting > 0;
}           // вернет true, если какая-то кнопка на клавиатуре нажата, иначе вернет false

char get_char_from_terminal() {
    struct termios oldt, newt;                  // состояния терминалов olddt - до, newt - после приема
    char ch;                                     // в нее запишется нажатая клавиша
    tcgetattr(STDIN_FILENO, &oldt);             //
    newt = oldt;                                // запомнили старый режим
    newt.c_lflag &= ~(ICANON |
                      ECHO);           // Во время приема необходим неканоничный ввод. Отключаем флаги "каноничный" и "эхо"
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);    // переключаемся в новый режим
    ch = getchar();                             // приняли клавишу
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);    // переключились в старый режим
    return ch;
}      // вернет нажатый символ

int height = 10;
int width = 2 * height;
int const_game_speed = 150000;
int game_speed = const_game_speed;

string status_game;

enum eDirection {
    STOP = 0, LEFT = 1, RIGHT = 2, UP = 3, DOWN = 4
};

//  в нем реализованы все действия, которые могут произойти со змеёй
class Snake {
    friend class Game;  // чтобы Game мог использовать приватные поля Snake
public:
    Snake() {
        dir = STOP;
        x = width / 2;
        y = height / 2;
        tail.emplace_back(make_pair(x, y));
    }

    void control() {
//        если кнопка нажата, то обрабатываем нажатие
        if (button_is_pressed()) {
            switch (get_char_from_terminal()) {
                case 'a':
                    if (dir != RIGHT)
                        dir = LEFT;
                    game_speed = const_game_speed;
                    break;
                case 'w':
                    if (dir != DOWN)
                        dir = UP;
                    game_speed = const_game_speed * 15 / 10;
                    break;
                case 'd':
                    if (dir != LEFT)
                        dir = RIGHT;
                    game_speed = const_game_speed;
                    break;
                case 's':
                    if (dir != UP)
                        dir = DOWN;
                    game_speed = const_game_speed * 15 / 10;
                    break;
                case 'x':
                    cout << "Pause press any key to continue..." << endl;
                    get_char_from_terminal();
                    cout << "Wait 3 seconds..." << endl;
                    sleep(3);
                    break;
                case 'o':
                    status_game = "Game over";
                    break;

            }
        }

//        сделаем так, чтобы змейка могла перемещаться
        switch (dir) {
            case LEFT:
                x--;
                break;
            case UP:
                y--;
                break;
            case DOWN:
                y++;
                break;
            case RIGHT:
                x++;
                break;
            case STOP:
                break;
        }

    }

    void growing() {
        tail.push_back(tail[tail.size() - 1]);
    }

private:
    int x, y;
    eDirection dir;
    vector<pair<int, int> >
            tail;
};

// интерактивное меню
class Menu {
    friend class Game;

public:
    Menu() {
        menu_pos = 1;
        setting_pos = 1;
        menu_status = "Main";
    }

    void menu() {
        while (status_game == "New game") {
            if (menu_status == "Main") {
                start_menu();
            } else if (menu_status == "Setting") {
                setting_menu();
            }

            if (button_is_pressed()) {
                switch (get_char_from_terminal()) {
                    case 'w':
                        if (menu_status == "Main") {
                            if (menu_pos > 1)
                                menu_pos--;
                        } else if (menu_status == "Setting") {
                            if (setting_pos > 1)
                                setting_pos--;
                        }
                        break;
                    case 's':
                        if (menu_status == "Main") {
                            if (menu_pos < 3)
                                menu_pos++;
                        } else if (menu_status == "Setting") {
                            if (setting_pos < 3)
                                setting_pos++;
                        }
                        break;
                    case '\n':
                        if (menu_status == "Main") {
                            if (menu_pos == 1)
                                return;
                            if (menu_pos == 2) {
                                menu_pos = 1;
                                menu_status = "Setting";
                            }
                            if (menu_pos == 3) {
                                system("clear");
                                exit(0);
                            }
                        } else if (menu_status == "Setting") {
                            if (setting_pos == 1) {
                                setting_pos = 1;
                                change_speed();
                                getchar();
                                continue;
                            }
                            if (setting_pos == 2) {
                                setting_pos = 1;
                                change_size_map();
                                getchar();
                                continue;
                            }
                            if (setting_pos == 3) {
                                setting_pos = 1;
                                menu_status = "Main";
                            }
                        }

                }
            }
            usleep(100000);     // задержка терминала
        }
    }

    void change_speed() {
        system("clear");
        cout << "################################################" << endl;
        cout << "#          Изменение скорости змейки           #" << endl;
        cout << "################################################" << endl;
        cout << "#   Текущая скорость змейки: " << setw(7) << left << game_speed << "           #" << endl;
        int new_speed = 0;
        cout << "#   Скорость меняется обратно пропорционально  #" << endl;
        cout << "#     -Легкая  скорость (медленно):  300'000   #" << endl;
        cout << "#     -Средняя скорость (нормально): 150'000   #" << endl;
        cout << "#     -Быстрая скорость (быстро):    100'000   #" << endl;
        cout << "#   Введите новую скорость:                    #" << endl;
        cout << "################################################" << endl;
        cin >> new_speed;
        game_speed = new_speed;
        const_game_speed = new_speed;
        return;
    }

    void change_size_map() {
        system("clear");
        cout << "################################################" << endl;
        cout << "#         Изменение рамера карты               #" << endl;
        cout << "################################################" << endl;
        cout << "#    Текущий размер карты: " << setw(3) << left << height << "                 #" << endl;
        cout << "#    Введите число от 2 до 30                  #" << endl;
        cout << "################################################" << endl;
        int new_height = 0;
        cin >> new_height;
        height = new_height;
        width = 2 * height;
        return;
    }

    void start_menu() {
        system("clear");
        switch (menu_pos) {
            case 1:
                cout << "################################################" << endl;
                cout << "#     Добро пожаловать в игру Snake v.1.0.0    #" << endl;
                cout << "################################################" << endl;
                cout << "# ->  1) Начать игру                           #" << endl;
                cout << "#     2) Настройки                             #" << endl;
                cout << "#     3) Выход                                 #" << endl;
                cout << "################################################" << endl;
                break;
            case 2:
                cout << "################################################" << endl;
                cout << "#     Добро пожаловать в игру Snake v.1.0.0    #" << endl;
                cout << "################################################" << endl;
                cout << "#     1) Начать игру                           #" << endl;
                cout << "# ->  2) Настройки                             #" << endl;
                cout << "#     3) Выход                                 #" << endl;
                cout << "################################################" << endl;
                break;
            case 3:
                cout << "################################################" << endl;
                cout << "#     Добро пожаловать в игру Snake v.1.0.0    #" << endl;
                cout << "################################################" << endl;
                cout << "#     1) Начать игру                           #" << endl;
                cout << "#     2) Настройки                             #" << endl;
                cout << "# ->  3) Выход                                 #" << endl;
                cout << "################################################" << endl;
                break;
        }
    }

    void setting_menu() {
        system("clear");
        switch (setting_pos) {
            case 1:
                cout << "################################################" << endl;
                cout << "#                  Настройки                   #" << endl;
                cout << "################################################" << endl;
                cout << "# ->  1) Изменить скорость змейки              #" << endl;
                cout << "#     2) Изменить размер карты                 #" << endl;
                cout << "#     3) Назад                                 #" << endl;
                cout << "################################################" << endl;
                break;
            case 2:
                cout << "################################################" << endl;
                cout << "#                  Настройки                   #" << endl;
                cout << "################################################" << endl;
                cout << "#     1) Изменить скорость змейки              #" << endl;
                cout << "# ->  2) Изменить размер карты                 #" << endl;
                cout << "#     3) Назад                                 #" << endl;
                cout << "################################################" << endl;
                break;
            case 3:
                cout << "################################################" << endl;
                cout << "#                  Настройки                   #" << endl;
                cout << "################################################" << endl;
                cout << "#     1) Изменить скорость змейки              #" << endl;
                cout << "#     2) Изменить размер карты                 #" << endl;
                cout << "# ->  3) Назад                                 #" << endl;
                cout << "################################################" << endl;
                break;
        }
    }

    void game_over(int score) {
        system("clear");
        cout << "################################################" << endl;
        cout << "#               Вы проиграли!                  #" << endl;
        cout << "################################################" << endl;
        cout << "#          Удачи в следующий раз!              #" << endl;
        cout << "#                Ваш счет: " << setw(5) << left << score << "               #" << endl;
        cout << "#               Хорошего дня!                  #" << endl;
        cout << "################################################" << endl;
        usleep(2000000);
        system("clear");
    }

    void game_win(int score) {
        system("clear");
        cout << "################################################" << endl;
        cout << "#                Вы выиграли!                  #" << endl;
        cout << "################################################" << endl;
        cout << "#          Удачи в следующий раз!              #" << endl;
        cout << "#                Ваш счет: " << setw(5) << left << score << "               #" << endl;
        cout << "#               Хорошего дня!                  #" << endl;
        cout << "################################################" << endl;
        usleep(2000000);
        system("clear");
    }

private:
    int menu_pos, setting_pos;
    string menu_status;
};

//  сама игра и её логика
class Game {
    friend Menu;
public:
    Game() {
        status_game = "New game";
        srand(time(nullptr));
        menu.menu();
        fruit_x = 5;
        fruit_y = 5;
        fruit_rand();
        score = 0;
    }

    void play() {
        while (status_game != "Game over") {
            print();            // рисуем карту
            snake.control();    // ловим перемещение
            logic();            // обрабатываем
            usleep(game_speed);     // задержка терминала
        }
        if (score == (height - 1) * (width - 1) * 10) {
            usleep(2000000);
            menu.game_win(score);
        } else {
            usleep(2000000);
            menu.game_over(score);
        }
    }

private:
    void fruit_rand() {
        while (true) {
            bool pos_fruit_in_tail = false;
            fruit_x = rand() % (width - 1) + 1;
            fruit_y = rand() % (height - 1) + 1;
//            usleep(100000);     // задержка терминала
            for (int i = 0; i < snake.tail.size(); i++) {
//                если фрукт появится в хвосте, то поставим флаг
                if (snake.tail[i] == make_pair(fruit_x, fruit_y)) {
                    pos_fruit_in_tail = true;
                    break;
                }
            }
//            если флаг стоит, то найдем новую позицию для фрукта
            if (!pos_fruit_in_tail) {
                break;
            }
            cout << snake.tail.size() * 10 << "   " << (height - 1) * (width - 1) * 10 << endl;
            if (snake.tail.size() * 10 == (height - 1) * (width - 1) * 10) {
                status_game = "Game over";
                break;
            }
        }
    }   // случайно выбирает позицию фрукта

    void print() {
        system("clear");
//        cout << "Score: " << score << endl;
//        cout << "Координаты фрукта: " << fruit_x << "   " << fruit_y << endl;
//        cout << "Координаты змейки: " << snake.x << "   " << snake.y << endl;
//        cout << "Скорость змейки: " << game_speed << endl;
//        cout << "Размер карты: " << height << endl;

        //        рисуем верхниюю часть стенки
        for (int i = 0; i < width + 1; i++) {
            cout << "#";
        }
        cout << endl;

//        рисуем поле и боковые стенки
        for (int i = 1; i < height; i++) {
            for (int j = 0; j < width + 1; j++) {
                if (j == 0 || j == width)
                    cout << "#";
                else if (i == fruit_y and j == fruit_x)
                    cout << "$";
                else if (i == snake.y and j == snake.x)
                    cout << "O";
                else {
                    bool print_tail = false;
                    for (int k = 0; k < snake.tail.size(); k++) {
                        if (snake.tail[k] == make_pair(j, i)) {
                            cout << "*";
                            print_tail = true;
                        }
                    }
                    if (!print_tail)
                        cout << " ";
                }
            }
            cout << endl;
        }

//        рисуем нижнюю часть стенки
        for (int i = 0; i < width + 1; i++) {
            cout << "#";
        }

        cout << endl;
    }       // тут рисуется карта, фрукт, змейка

    void logic() {
//        прошли сквозь стену
        if (snake.x <= 0) {
            snake.x = width - 1;
        }
        if (snake.x >= width) {
            snake.x = 1;
        }
        if (snake.y <= 0) {
            snake.y = height - 1;
        }
        if (snake.y >= height) {
            snake.y = 1;
        }

//        если змейка съела фрукт
        if (snake.x == fruit_x and snake.y == fruit_y) {
            score += 10;

            fruit_rand();
            snake.growing();
        }

//       обновим хвост
        pair<int, int> prev = snake.tail[0];
        pair<int, int> prev2;
        snake.tail[0] = make_pair(snake.x, snake.y);
        for (int i = 1; i < snake.tail.size(); i++) {
            prev2 = snake.tail[i];
            snake.tail[i] = prev;
            prev = prev2;
        }

//        проверим на пересечение с хвостом
        for (int i = 0; i < snake.tail.size(); i++) {
            if (snake.tail[0] == snake.tail[i] and i != 0)
                status_game = "Game over";
        }
    }

    Snake snake;
    Menu menu;
    int fruit_x, fruit_y, score;
};

int main() {
    Game game;
    game.play();
    return 0;
}
