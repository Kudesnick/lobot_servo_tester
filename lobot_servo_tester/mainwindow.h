#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_btn_connect_uart_clicked();

    void on_btn_refresh_uart_list_clicked();

    void on_btn_close_uart_clicked();

    void on_btn_SERVO_ID_READ_clicked();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
