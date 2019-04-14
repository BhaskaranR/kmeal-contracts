#include "config.hpp"
#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/print.hpp>
#include <eosiolib/time.hpp>

#include <string>
#include <vector>

using namespace eosio;
using namespace std;

CONTRACT kmeal : public contract {
     
  private:
    struct [[eosio::table, eosio::contract("kmeal")]] account
    {
        name owner;
        asset balance = asset{0, DEFAULT_SYMBOL};
        
        uint64_t primary_key() const { return owner.value; }
    };
    
    typedef eosio::multi_index<"accounts"_n, account> accounts_table;

    struct [[eosio::table, eosio::contract("kmeal")]]  restaurant
    {
        uint64_t restaurant_id;
        name owner;
        string name;
        string description;
        string phone;
        uint8_t rating;
        string address;
        string address2;
        string city;
        string state;
        string postalCode;
        double latitude;
        double longitude;
        string logo;
        string timeofoperation;
        vector<string> categories;
        uint8_t is_active;
        
        uint64_t primary_key() const { return owner.value; }
    
        // EOSLIB_SERIALIZE(restaurant, (owner)(name)(description)(phone)(rating)(address)(address2)(city)(state)(postalCode)(latitude)(longitude)(logo)(timeofoperation)(categories)(is_active))
    };

    typedef multi_index<"restaurants"_n, restaurant
    > restaurants_table;
    
    struct [[eosio::table, eosio::contract("kmeal")]]  deposit {
        name owner;
        asset balance;

        uint64_t primary_key() const { return owner.value; }
    };

    typedef multi_index<"deposits"_n, deposit> deposit_table;
    
    struct item {
      name             owner;
      uint64_t         item_id;
      string           item_name;
      string           description;
      string           photo;
      uint64_t         spicy_level;
      uint64_t         vegetarian;
      uint64_t         cooking_time;
      uint64_t         is_active;
      vector<string>   types;
      uint64_t         primary_key() const { return item_id; }
      
      //secondary key index todo
      //EOSLIB_SERIALIZE(item, (item_id)(item_name)(description)(photo)(spicy_level)(vegetarian)(cooking_time)(types))
    };
    
    typedef multi_index<"items"_n, item> items_table;
    
    struct section {
      uint64_t         section_id;
      string           section_name;
      vector<uint64_t> items;
      uint64_t         primary_key() const { return section_id; }
    };
    
    struct [[eosio::table, eosio::contract("kmeal")]]  book {
      uint64_t        book_id;
      name            owner;
      string          book_name;
      vector<section> sections;
      
      uint64_t         primary_key() const { return book_id; }
    };
    
    typedef multi_index<"books"_n, book> books_table;
    
    struct listing_sides {
      uint64_t   item_id;
      string     group;
      uint64_t   max_selection;
      float      list_price;
    };
    
    struct  [[eosio::table, eosio::contract("kmeal")]]  listing {
      name         owner; 
      uint64_t     listing_id;
      char         list_type;
      uint64_t     book_id;
      uint64_t     item_id;
      uint64_t     section_id;
      float        list_price;
      float        min_price;
      uint64_t     quantity;
      uint32_t     duration = 0; // event duration
      uint64_t     sliding_rate;
      time_point_sec expires; 
      uint64_t     status;
      vector<listing_sides>   sides;
      bool         isactive;
      
      uint64_t     primary_key() const { return listing_id; }
      
      uint64_t get_expires() const { return expires.utc_seconds; }
      uint128_t get_price() const { return list_price; }

    };
    
    typedef multi_index<"listings"_n, listing,
      indexed_by<"expires"_n, const_mem_fun<listing, uint64_t, &listing::get_expires>>,
      indexed_by< "byprice"_n, const_mem_fun< listing, uint128_t, &listing::get_price>>
    > listings_table;
    
    struct  [[eosio::table, eosio::contract("kmeal")]]  orderdetail { 
      uint64_t  order_detail_id;
      uint64_t  listing_id;
      uint64_t  quantity;
      float     ordered_price;
      float     final_price;
      char      listing_type;
      string    instructions;
      
      uint64_t  primary_key() const { return order_detail_id; }
      uint64_t  by_listing_id() const { return listing_id; }
    };
    
  typedef multi_index<"orderdetails"_n, orderdetail, 
    indexed_by<"bylistingid"_n,
        const_mem_fun<orderdetail, uint64_t, &orderdetail::by_listing_id>>> orderdetail_table;
  
  struct  [[eosio::table, eosio::contract("kmeal")]]  order {
    uint64_t   order_id;
    name       buyer; 
    name       seller;
    uint64_t   order_status;
    string     instructions;
    vector<uint64_t> detail;
    uint64_t    primary_key() const { return order_id; }
    uint64_t    by_buyer() const { return buyer.value; }
  };
    
  typedef multi_index<"orders"_n, order, 
     indexed_by<"bybuyer"_n,  const_mem_fun<order, uint64_t, &order::by_buyer>>> order_table;
    
  void transfer_kmeal(name from, name to, asset quantity, std::string memo);
    
  items_table items;
  deposit_table deposits;
  books_table books;
  listings_table listings;
  restaurants_table restaurants;
  accounts_table accounts;
  order_table    orders;
  orderdetail_table orderdetails;
    
  public:
    
    kmeal(eosio::name self, eosio::name code, datastream<const char*> ds)
    : contract(self, code, ds),
      deposits(self, self.value),
      accounts(self, self.value),
      items(self, self.value),
      books(self, self.value),
      listings(self, self.value),
      restaurants(self,self.value),
      orders(self, self.value)  ,
      orderdetails(self, self.value)
      {}
      
    template <typename T>
    void cleanTable(){
        T db(_self, _self.value);
        
        while(db.begin() != db.end()){
                auto itr = --db.end();
                db.erase(itr);
        }
    }
    
    ACTION cleartables();
    
    ACTION setuprest(const name account, 
                    const string name, 
                    const string description, 
                    const string phone, 
                    const string address,
                    const string address2,
                    const string city,
                    const string state,
                    const string postalCode,
                    const double latitude, 
                    const double longitude, 
                    const string logo, 
                    const vector<string> categories, 
                    const string timeofoperation);
                    
    ACTION delrest(const name account);

    ACTION createbook(const name account, const string bookname);
    
    ACTION addsections(uint64_t bookid, std::string sectionname, uint64_t sortorder);
    
    ACTION editsec(uint64_t bookid, std::string sectionname, uint64_t sortorder);
      
    ACTION setsecorder(uint64_t bookid, uint64_t sectionid, uint64_t sortorder);
    
    ACTION delsec(uint64_t bookid, uint64_t secid);
    
    ACTION createitem(name account,
      string    itemname,
      string    description,
      string    photo,
      uint64_t  spicy_level,
      uint64_t  vegetarian,
      uint64_t  cooking_time,
      vector<string> types);
    
    ACTION edititem(
      uint64_t  itemid,
      string    itemname,
      string    description,
      string    photo,
      uint64_t  spicy_level,
      uint64_t  vegetarian,
      uint64_t  cooking_time,
      vector<string> types);
    
    ACTION addtosection(uint64_t bookid, uint64_t sectionid, uint64_t  itemid, uint64_t  sortorder);
    
    ACTION listitem(
      uint64_t     book_id,
      uint64_t     item_id,
      uint64_t     section_id,
      char         list_type,
      float        list_price,
      float        min_price,
      uint64_t     quantity,
      uint32_t     duration, // event duration
      uint32_t     expires,    
      uint64_t     sliding_rate,
      uint64_t     status,
      vector<listing_sides>   sides,
      bool         isactive
      );
    
    ACTION placeorder( uint64_t  order_id, name  buyer, name seller,   uint64_t order_status, string  instructions, vector<orderdetail> detail);
    
    ACTION opendeposit(name owner);

    ACTION closedeposit(name owner);

    ACTION depositkmeal(name from, name to, asset quantity, std::string memo);
 
};