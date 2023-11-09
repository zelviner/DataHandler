#include <iostream>
#include <string>

// 抽象产品：读卡器
class CardReader {
public:
    virtual void open() = 0;
    virtual void getATR() = 0;
    virtual void close() = 0;
};

// 具体产品1：PCSC读卡器
class PCSCReader : public CardReader {
public:
    void open() override {
        std::cout << "PCSC reader opened" << std::endl;
        // 连接到PCSC读卡器的具体代码
    }

    void getATR() override {
        std::cout << "PCSC reader gets ATR" << std::endl;
        // 获取ATR的具体代码
    }

    void close() override {
        std::cout << "PCSC reader closed" << std::endl;
        // 关闭PCSC读卡器的具体代码
    }
};

// 具体产品2：Smartware读卡器
class SmartwareReader : public CardReader {
public:
    void open() override {
        std::cout << "Smartware reader opened" << std::endl;
        // 连接到Smartware读卡器的具体代码
    }

    void getATR() override {
        std::cout << "Smartware reader gets ATR" << std::endl;
        // 获取ATR的具体代码
    }

    void close() override {
        std::cout << "Smartware reader closed" << std::endl;
        // 关闭Smartware读卡器的具体代码
    }
};

// 工厂方法接口
class CardReaderFactory {
public:
    virtual CardReader* createCardReader() = 0;
};

// 具体工厂1：PCSC读卡器工厂
class PCSCReaderFactory : public CardReaderFactory {
public:
    CardReader* createCardReader() override {
        return new PCSCReader();
    }
};

// 具体工厂2：Smartware读卡器工厂
class SmartwareReaderFactory : public CardReaderFactory {
public:
    CardReader* createCardReader() override {
        return new SmartwareReader();
    }
};

int main() {
    // 创建PCSC读卡器工厂
    CardReaderFactory* pcscFactory = new PCSCReaderFactory();

    // 使用PCSC读卡器工厂创建PCSC读卡器对象
    CardReader* pcscReader = pcscFactory->createCardReader();

    // 执行PCSC读卡器操作
    pcscReader->open();
    pcscReader->getATR();
    pcscReader->close();

    // 释放PCSC资源
    delete pcscReader;
    delete pcscFactory;

    // 创建Smartware读卡器工厂
    CardReaderFactory* smartwareFactory = new SmartwareReaderFactory();

    // 使用Smartware读卡器工厂创建Smartware读卡器对象
    CardReader* smartwareReader = smartwareFactory->createCardReader();

    // 执行Smartware读卡器操作
    smartwareReader->open();
    smartwareReader->getATR();
    smartwareReader->close();

    // 释放Smartware资源
    delete smartwareReader;
    delete smartwareFactory;












// 2 1
// 3 0 
// 1
// 1 读卡器数量
// B5 读卡器编号 181
// 51 83 CE C1 

// 2 1 
// 3 0
// 1
// 1读卡器数量
// B6 读卡器编号 182
// F9 EF DE 88













    return 0;
}
