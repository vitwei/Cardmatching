#pragma once
#ifndef CardDB
#define CardDB

#include <iostream>
#include <string>
#include <vector>


using namespace std;

class Basecard {
public:
    // Default constructor
    Basecard();

    Basecard(const std::string& img,
        const std::string& textcontent,
        const std::vector<float>& price,
        const std::string& createtime,
        const std::string& updatetime,
        const std::vector<std::string>& pricetime
    );
    // Destructor
    ~Basecard();

    // Setter functions
    void setImg(const std::string& img);
    void setTextContent(const std::string& textcontent);
    void setPrice(const std::vector<float>& price);
    void setPriceTime(const std::vector<std::string>& pricetime);
    void setCreateTime(const std::string& createtime);
    void setUpdateTime(const std::string& updatetime);

    // Getter functions
    std::string getImg() const;
    std::string getTextContent() const;
    std::vector<float> getPrice() const;
    std::vector<std::string> getPriceTime() const;
    std::string getCreateTime() const;
    std::string getUpdateTime() const;

private:
    std::string img;
    std::string textcontent;
    std::vector<float> price;
    std::vector<std::string> pricetime;
    std::string createtime;
    std::string updatetime;
};

// Default constructor


class carddata {
public:
    // Default constructor
    carddata();

    carddata(const std::string& img,
        const std::string& textcontent,
        float price,
        int bidcount,
        const std::string& dealtime);


    // Destructor
    ~carddata();

    // Setter functions
    void setImg(const std::string& img);
    void setTextContent(const std::string& textcontent);
    void setPrice(float price);
    void setBidCount(int bidcount);
    void setDealTime(const std::string& dealtime);

    // Getter functions
    std::string getImg() const;
    std::string getTextContent() const;
    float getPrice() const;
    int getBidCount() const;
    std::string getDealTime() const;

private:
    std::string img;
    std::string textcontent;
    float price;
    int bidcount;
    std::string dealtime;
};


#endif 