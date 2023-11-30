#include <QtCore/QCoreApplication>
#include <QThread>
#include <stdio.h>
class MyThread:public QThread
{
    Q_OBJECT
public:
    MyThread();
    void stop();
private:
    bool isRunning;
    void run();
public slots:
    void showMsg();
signals:
    void msg();
};
MyThread::MyThread()
{
    isRunning = true;
    connect(this,SIGNAL(msg()),this,SLOT(showMsg()),Qt::DirectConnection);
}
void MyThread::showMsg()
{
    printf("Hello!\n");
}
void MyThread::run()
{
    while(isRunning)
    {
        sleep(1);
        emit msg();
    }
    printf("Exit!\n");
}
void MyThread::stop()
{
    isRunning = false;
}
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    MyThread mThread;
    mThread.start();

    // 计时 - 开始
    auto start = std::chrono::steady_clock::now();
 
    while(1)
    {
        if(getchar()=='B')
        {
            mThread.stop();
            mThread.wait();
            break;
        }
    }

    // 计时 - 结束
    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed_seconds = end - start;
    return a.exec();
}
// #include "main.moc"
 