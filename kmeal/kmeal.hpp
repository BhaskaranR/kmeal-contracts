#include "config.hpp"
#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/print.hpp>
#include <eosiolib/time.hpp>
#include <eosiolib/action.hpp>
#include <eosiolib/multi_index.hpp>
#include <eosiolib/crypto.h>
#include <eosiolib/time.hpp>
#include <eosiolib/transaction.hpp>

#include <string>
#include <vector>

using namespace eosio;
using namespace std;
using std::to_string;

CONTRACT kmeal : public contract
{

public:
  const uint16_t BUYER_ORDERED_FLAG = 1 << 0;
  const uint16_t SELLER_ACCEPTED_FLAG = 1 << 1;
  const uint16_t ORDER_FUNDED_FLAG = 1 << 2;
  const uint16_t ORDER_DELIVERED_FLAG = 1 << 3;
  const uint16_t ORDER_ARBITRATION_FLAG = 1 << 4;

  const uint16_t BOTH_ACCEPTED_FLAG = BUYER_ORDERED_FLAG | SELLER_ACCEPTED_FLAG;

  const uint16_t DYNAMIC_LIST_TYPE_FLAG = 1 << 0;
  const uint16_t REGULAR_LIST_TYPE_FLAG = 1 << 1;
  const uint16_t GROUP_LIST_TYPE_FLAG = 1 << 2;

  const uint16_t DYNAMIC_ORDER_FLAG = 1 << 0;
  const uint16_t GROUP_ORDER_FLAG = 1 << 1;
  const uint16_t REGULAR_ORDER_FLAG = 1 << 2;

private:


  struct transfer
  {
    name         from;
    name         to;
    asset        quantity;
    string       memo;
  };


  struct order_notification_abi {
    name        order_status;
    string      message;
    uint64_t    order_id;
    string      description;
    name        tkcontract;
    asset       quantity;
    name        buyer;
    name        seller;
    name        arbiter;
    uint32_t    days;
    string      memo;
  };

  TABLE account
  {
    name owner;
    asset balance;

    uint64_t primary_key() const { return owner.value; }
  };

  typedef eosio::multi_index<"accounts"_n, account> accounts_table;

  TABLE restaurant
  {
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

  };

  typedef multi_index<"restaurants"_n, restaurant> restaurants_table;

  TABLE arbiter
  {
    name account;
    string contact_name;
    string email;
    string description;
    string website;
    string phone;
    string iso_country;
    uint32_t processed_deals = 0;
    uint8_t is_active;

    auto primary_key() const { return account.value; }
    uint64_t get_is_active() const { return is_active; }
  };

  typedef eosio::multi_index<
      name("arbiters"), arbiter,
      indexed_by<name("active"), const_mem_fun<arbiter, uint64_t, &arbiter::get_is_active>>>
      arbiters_table;

  TABLE deposit
  {
    name owner;
    asset balance;

    uint64_t primary_key() const { return owner.value; }
  };

  typedef multi_index<"deposits"_n, deposit> deposit_table;

  TABLE item
  {
    name owner;
    uint64_t item_id;
    string item_name;
    string description;
    string photo;
    uint64_t spicy_level;
    uint64_t vegetarian;
    uint64_t cooking_time;
    uint8_t is_active;
    vector<string> types;
    
    uint64_t primary_key() const { return item_id; }

    uint64_t get_owner()const  { return  owner.value; }    // 2
    
  };

  typedef multi_index<"items"_n, item,
                    indexed_by<"itembyowner"_n,    const_mem_fun<item, uint64_t, &item::get_owner>>> items_table;

  TABLE sec
  {
    name owner;
    uint64_t section_id;
    string section_name;
    vector<uint64_t> items;
    uint8_t is_active;
    uint64_t primary_key() const { return section_id; }
    
    
    uint64_t get_owner()const  { return  owner.value; }    // 2
  };
  
  
  typedef multi_index<"sec"_n, sec,
  indexed_by<"secbyowner"_n,    const_mem_fun<sec, uint64_t, &sec::get_owner>>
  > sec_table;

  TABLE book
  {
    uint64_t book_id;
    name owner;
    string book_name;
    uint8_t is_active;
    
    vector<uint64_t> sections;

    uint64_t primary_key() const { return book_id; }
    
    uint64_t get_owner()const  { return  owner.value; }  
  };

  typedef multi_index<"books"_n, book,
          indexed_by<"booksbyowner"_n,    const_mem_fun<book, uint64_t, &book::get_owner>>
  > books_table;

  struct listing_sides
  {
    uint64_t item_id;
    string group;
    uint64_t max_selection;
    float list_price;
  };

  TABLE listing
  {
    name owner;
    uint64_t listing_id;
    uint64_t list_type;
    uint64_t book_id;
    uint64_t item_id;
    uint64_t section_id;
    //uint8_t is_active;
    float list_price;
    float min_price;
    uint64_t quantity;
    uint64_t sliding_rate;
    time_point_sec expires;
    uint64_t status;
    vector<listing_sides> sides;
    bool isactive;

    uint64_t primary_key() const { return listing_id; }

    uint64_t get_expires() const { return expires.utc_seconds; }

    uint128_t get_price() const { return list_price; }
    
    uint64_t get_itemid() const { return item_id; }
    
    uint64_t get_owner()const  { return  owner.value; }  
    
    uint64_t get_listtype() const  { return  list_type; }
    
  };
  

  typedef multi_index<"listings"_n, listing,
                      indexed_by<"byowner"_n,    const_mem_fun<listing, uint64_t, &listing::get_owner>>,
                      indexed_by<"bylisttype"_n, const_mem_fun<listing, uint64_t, &listing::get_listtype>>,
                      indexed_by<"byitemid"_n, const_mem_fun<listing, uint64_t, &listing::get_itemid>>,
                      indexed_by<"expires"_n, const_mem_fun<listing, uint64_t, &listing::get_expires>>,
                      indexed_by<"byprice"_n, const_mem_fun<listing, uint128_t, &listing::get_price>>
                      >
      listings_table;

  TABLE orderdetail
  {
    uint64_t order_detail_id;
    uint64_t listing_id;
    uint64_t quantity;
    float ordered_price;
    float final_price;
    string instructions;

    uint64_t primary_key() const { return order_detail_id; }

    uint64_t by_listing_id() const { return listing_id; }
  };

  typedef multi_index<"orderdetails"_n, orderdetail,
                      indexed_by<"bylistingid"_n, const_mem_fun<orderdetail, uint64_t, &orderdetail::by_listing_id>>>
      orderdetail_table;

  TABLE order
  {
    uint64_t order_id;
    name buyer;
    name seller;
    name arbitrator;
    uint64_t order_status;
    uint16_t flags;
    uint64_t order_type;
    asset total_price;
    string instructions;
    uint32_t    days;
    vector<uint64_t> detail;
    time_point_sec funded;
    time_point_sec expires;
    
    string delivery_memo;
    

    uint64_t primary_key() const { return order_id; }
    uint64_t by_buyer() const { return buyer.value; }

    uint64_t get_expires() const { return expires.utc_seconds; }

    uint64_t get_order_type() const { return order_type; }
  };

  typedef multi_index<"orders"_n, order,
                      indexed_by<"bybuyer"_n, const_mem_fun<order, uint64_t, &order::by_buyer>>,
                      indexed_by<"byordertype"_n, const_mem_fun<order, uint64_t, &order::get_order_type>>>
      order_table;


  items_table items;
  deposit_table deposits;
  sec_table sections;
  books_table books;
  listings_table listings;
  restaurants_table restaurants;
  accounts_table accounts;
  order_table orders;
  orderdetail_table orderdetails;
  arbiters_table arbiters;

public:
  kmeal(eosio::name self, eosio::name code, datastream<const char *> ds)
      : contract(self, code, ds),
        deposits(self, self.value),
        accounts(self, self.value),
        items(self, self.value),
        sections(self, self.value),
        books(self, self.value),
        listings(self, self.value),
        restaurants(self, self.value),
        arbiters(self, self.value),
        orders(self, self.value),
        orderdetails(self, self.value)
  {
    
  }
  
  
  void send_payment( name to, asset quantity, std::string memo);
  
  void transfer_handler(name from, name to, asset quantity, std::string memo);

  template <typename T>
  void cleanTable()
  {
    T db(_self, _self.value);

    while (db.begin() != db.end())
    {
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
  
  void checkOrdersForListings(uint64_t listingid) ;
  
  void checkListingsForItem(uint64_t itemid);
  
  void checkItemsForSec(uint64_t sectionid);
  
  void checkSectionsForBook(uint64_t bookid);
  
  ACTION delbook(const uint64_t bookid);

  ACTION addsections(uint64_t bookid, std::string sectionname);

  // ACTION editsec(uint64_t bookid, std::string sectionname, uint64_t sortorder);

  ACTION setsecorder(uint64_t bookid, uint64_t sectionid, uint64_t sortorder);

  ACTION delsec(uint64_t bookid, uint64_t secid);

  ACTION createitem(
                    name account,
                    string itemname,
                    string description,
                    string photo,
                    uint64_t spicy_level,
                    uint64_t vegetarian,
                    uint64_t cooking_time,
                    vector<string> types,
                    uint64_t book_id = -1,
                    uint64_t section_id = -1);
                    

  ACTION delitem(uint64_t itemid);

  ACTION edititem(uint64_t itemid, string itemname, string description, string photo, uint64_t spicy_level, uint64_t vegetarian, uint64_t cooking_time, vector<string> types);
  

  ACTION addtosection(uint64_t bookid, uint64_t sectionid, uint64_t itemid, uint64_t sortorder);
  
  void addItemToSection(uint64_t bookid, uint64_t sectionid, uint64_t itemid, uint64_t sortorder);
  
  ACTION removefromsec(uint64_t sectionid, uint64_t itemid);
  
  ACTION listitem(
      uint64_t book_id,
      uint64_t item_id,
      uint64_t section_id,
      uint64_t list_type,
      float list_price,
      float min_price,
      uint64_t quantity,
      uint32_t expires,
      float sliding_rate,
      vector<listing_sides> sides);
      
  ACTION deletelisting(uint64_t listing_id);

  ACTION placeorder(name buyer, name seller, string instructions, vector<orderdetail> detail);

  ACTION accept(name seller, uint64_t order_id);

  ACTION cancel(uint64_t order_id);

  ACTION delivered(uint64_t order_id, string memo);

  ACTION goodsrcvd(uint64_t order_id);

  ACTION opendeposit(name owner);

  ACTION closedeposit(name owner);

  ACTION depositkmeal(name from, name to, asset quantity, std::string memo);

  ACTION notify(name order_status, uint64_t order_id, string description, asset quantity,
                name buyer, name seller, name arbiter, uint32_t days, string memo);

  ACTION setarbiter(name account, string contact_name, string email, string description,
                    string website, string phone, string iso_country);

  ACTION delarbiter(name account);
};