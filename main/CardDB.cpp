#include "CardDB.hpp"

Basecard::Basecard() {
    // Initialize members as needed
}

// Destructor
Basecard::~Basecard() {
    // Cleanup resources if necessary
}

// Setter functions
void Basecard::setImg(const std::string& img) {
    this->img = img;
}

void Basecard::setTextContent(const std::string& textcontent) {
    this->textcontent = textcontent;
}

void Basecard::setPrice(const std::vector<float>& price) {
    this->price = price;
}

void Basecard::setPriceTime(const std::vector<std::string>& pricetime) {
    this->pricetime = pricetime;
}

void Basecard::setCreateTime(const std::string& createtime) {
    this->createtime = createtime;
}

void Basecard::setUpdateTime(const std::string& updatetime) {
    this->updatetime = updatetime;
}

// Getter functions
std::string Basecard::getImg() const {
    return img;
}

std::string Basecard::getTextContent() const {
    return textcontent;
}

std::vector<float> Basecard::getPrice() const {
    return price;
}

std::vector<std::string> Basecard::getPriceTime() const {
    return pricetime;
}

std::string Basecard::getCreateTime() const {
    return createtime;
}

std::string Basecard::getUpdateTime() const {
    return updatetime;
}

Basecard::Basecard(const std::string& img,
    const std::string& textcontent,
    const std::vector<float>& price,
    const std::string& createtime,
    const std::string& updatetime,
    const std::vector<std::string>& pricetime
)
    : img(img),
    textcontent(textcontent),
    price(price),
    createtime(createtime),
    updatetime(updatetime),
    pricetime(pricetime)
{
    // Additional initialization if necessary
}
// Default constructor
carddata::carddata() {
    // Initialize members as needed
}

// Destructor
carddata::~carddata() {
    // Cleanup resources if necessary
}

// Setter functions
void carddata::setImg(const std::string& img) {
    this->img = img;
}

void carddata::setTextContent(const std::string& textcontent) {
    this->textcontent = textcontent;
}

void carddata::setPrice(float price) {
    this->price = price;
}

void carddata::setBidCount(int bidcount) {
    this->bidcount = bidcount;
}

void carddata::setDealTime(const std::string& dealtime) {
    this->dealtime = dealtime;
}

// Getter functions
std::string carddata::getImg() const {
    return img;
}

std::string carddata::getTextContent() const {
    return textcontent;
}

float carddata::getPrice() const {
    return price;
}

int carddata::getBidCount() const {
    return bidcount;
}

std::string carddata::getDealTime() const {
    return dealtime;
}

carddata::carddata(const std::string& img,
    const std::string& textcontent,
    float price,
    int bidcount,
    const std::string& dealtime)
    : img(img), textcontent(textcontent), price(price), bidcount(bidcount), dealtime(dealtime) {
    // Additional initialization if necessary
}
