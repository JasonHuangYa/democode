class noncopyable{
    protected:
        noncopyable(){}
        ~ noncopyable(){}
    protected:
        noncopyable(const noncopyable &);
        const noncopyable& operator=(const noncopyable&);
    

};
