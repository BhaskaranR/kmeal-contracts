#include "kmeal.hpp"


using namespace eosio;
using namespace std;

    
void kmeal::cleartables()
{
    require_auth(_self);
    
    eosio::print(eosio::name(_self.value));
    auto restaurants_itr = restaurants.begin();
    while (restaurants_itr != restaurants.end()) {
        restaurants_itr = restaurants.erase (restaurants_itr);
    }
      
    auto accounts_itr = accounts.begin();
    while (accounts_itr != accounts.end()) {
        accounts_itr = accounts.erase (accounts_itr);
    }
      
    auto books_itr = books.begin();
    while (books_itr != books.end()) {
      books_itr = books.erase (books_itr);
    }
      
    auto orders_itr = orders.begin();
    while (orders_itr != orders.end()) {
        orders_itr = orders.erase (orders_itr);
    }
    
    auto listings_itr = listings.begin();
    while (listings_itr != listings.end()) {
        listings_itr = listings.erase (listings_itr);
    }
    
    auto deposits_itr = deposits.begin();
    while (deposits_itr != deposits.end()) {
        deposits_itr = deposits.erase (deposits_itr);
    }
    
    auto items_itr = items.begin();
    while (items_itr != items.end()) {
        items_itr = items.erase (items_itr);
    }
}


void kmeal::setuprest(const name account, 
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
                    const string timeofoperation
                    )
{
    require_auth(account);
    eosio_assert(is_account(account), "account does not exist");
    eosio_assert(name.length() > 0, "name cannot be empty");
    eosio_assert(phone.length() > 0, "phone cannot be empty");
    //todo
    // eosio_assert(address.length() > 0, "address cannot be empty");
    // eosio_assert(address2.length() > 0, "address cannot be empty");
    
    // auto res_acc_index = restaurants.get_index<"owner"_n>();
    // auto iter = res_acc_index.find(account.value);
    // eosio_assert(iter == res_acc_index.end(), "already signed up" );
    
    
    //assign permission for owner to onboard..
    
    auto accsetter = [&]( auto& s ) {
        s.owner = account;
        s.is_active = 1;
    };
    
    auto setter = [&]( auto& s ) {
    s.owner = account;
        s.name = name;
        s.description = description;
        s.phone = phone;
        s.address = address;
        s.address2 = address2;
        s.city = city;
        s.state = state;
        s.postalCode = postalCode;
        s.latitude = latitude;
        s.longitude = longitude;
        s.logo = logo;
        s.categories = categories;
        s.timeofoperation = timeofoperation;
    };
    
    auto _restaurant = restaurants.find(account.value);
    if( _restaurant == restaurants.end() ) {
        auto _account = accounts.find(account.value);
        eosio_assert(_account != accounts.end(), "already signed up as customer" );
        auto itr = accounts.find(account.value);
        accounts.emplace( _self, [&]( auto& a ){
              a.owner = account;
              a.balance = asset{0, DEFAULT_SYMBOL};
        });
        restaurants.emplace(account, setter);
    }
    else {
      restaurants.modify(*_restaurant, account, setter);
    }
}

  
void kmeal::delrest(name account)
{
  require_auth(account);
  auto _restaurant = restaurants.find(account.value);
  eosio_assert(_restaurant != restaurants.end(), "Cannot find the restaurant");
  eosio_assert(_restaurant->is_active, "This restaurant is already marked for deletion");
  //todo cannot delete during time of operation and cannot delete when you have active orders
  restaurants.modify(*_restaurant, account, [&]( auto& item ) {
      item.is_active = 0;
    });
}


void kmeal::createbook(const name account, const string bookname) {
  require_auth(account);
  
  auto iter = restaurants.find(account.value);
  eosio_assert(iter != restaurants.end(), "restaurant owner not found");
  
  auto _books = books.find(account.value);
  if( _books == books.end() ) { 
    books.emplace(_self, [&]( auto& s ) {
      s.book_id = books.available_primary_key();
      s.owner = account;
      s.book_name = bookname;
      s.sections =  vector<kmeal::section>();
    });
  }
}

void kmeal::addsections(uint64_t bookid, std::string sectionname, uint64_t sortorder) {
  auto iter = books.find(bookid);
  eosio_assert(iter != books.end(), "bookid not found");
  require_auth(iter->owner);
  auto sections = iter->sections;
  //check if section name is already available
  section sec;
  sec.section_name = sectionname;
  sec.items = vector<uint64_t>();
  sections.push_back(sec);
}

void kmeal::setsecorder(uint64_t bookid, uint64_t sectionid,  uint64_t sortorder) {
  auto iter = books.find(bookid);
  eosio_assert(iter != books.end(), "bookid not found");
  require_auth(iter->owner);
  vector<section> sections = iter->sections;
  eosio_assert(sections.cbegin() != sections.cend(), "no section values yet");
  auto length = sections.size();
  eosio_assert(sortorder > length, "wrong sort order");
  
  for(int index = 1; index < length; index++) {
    if (sections[index].section_id == sectionid ) {
        auto section = sections[index];
        sections.erase(sections.begin() + index);
        sections.insert(sections.begin() + sortorder, section);
        break;
    }
  }
}

void kmeal::delsec(uint64_t bookid, uint64_t secid) {
  auto iter = books.find(bookid);
  eosio_assert(iter != books.end(), "bookid not found");
  require_auth(iter->owner);
  vector<section> sections = iter->sections;
  eosio_assert(sections.cbegin() != sections.cend(), "no section values yet"); 
    
  auto length = sections.size();
  for(int index = 1; index < length; index++) {
      if (sections[index].section_id == secid ) {
        sections.erase(sections.begin() + index);
        break;
      }
  }
}

void kmeal::createitem(name account,
                      string    itemname,
                      string    description,
                      string    photo,
                      uint64_t  spicy_level,
                      uint64_t  vegetarian,
                      uint64_t  cooking_time,
                      vector<string> types) {
  //todo  validate other inputs
    auto iter = restaurants.find(account.value);
    eosio_assert(iter != restaurants.end(), "restaurant owner not found");
    require_auth (iter->owner);
    
    auto _items = items.find(account.value);
    //find itemid by secoundary indx which is the item namespace
    if( _items == items.end() ) {
      items.emplace(_self, [&]( auto& s) {
        s.item_id = items.available_primary_key();
        s.owner = account;
        s.item_name = itemname;
        s.description = description;
        s.photo = photo;
        s.spicy_level = spicy_level;
        s.vegetarian = vegetarian;
        s.cooking_time = cooking_time;
        s.types = types;
        s.is_active = 1;
      });
    }
  }

void kmeal::edititem(
      uint64_t  itemid,
      string    itemname,
      string    description,
      string    photo,
      uint64_t  spicy_level,
      uint64_t  vegetarian,
      uint64_t  cooking_time,
      vector<string> types) {
        
      auto _item = items.find(itemid);
      eosio_assert(_item != items.end(), "item could not be found");
      require_auth(_item->owner);
    
      items.modify(*_item, _self, [&]( auto& s ) {
        s.item_name = itemname;
        s.description = description;
        s.photo = photo;
        s.spicy_level = spicy_level;
        s.vegetarian = vegetarian;
        s.cooking_time = cooking_time;
        s.types = types;
        s.is_active = 1;
      });
    
  }

void kmeal::addtosection(uint64_t bookid, uint64_t sectionid, uint64_t  itemid, uint64_t  sortorder) {
  //auth the owner of bookid
  auto iter = books.find(bookid);
  eosio_assert(iter != books.end(), "bookid not found");
  require_auth(iter->owner);
  
  //auth the owner of itemid
  auto i_iter = items.find(itemid);
  eosio_assert(i_iter != items.end(), "itemid not found");
  require_auth(i_iter->owner);
  //loop through sections
  auto sections = iter->sections;
  for (int r = 0; r < sections.size(); r++) {
    if(sections[r].section_id == sectionid) {
        vector<uint64_t> items =sections[r].items;
        auto itemid_itr = std::find (items.begin(), items.end(), itemid);
        if (itemid_itr != items.end()) {
          if (sortorder > items.size()){
            items.push_back(itemid);
          } else { 
            items.insert(items.begin() + sortorder, itemid);
          }
        } else {
          items.erase(items.begin() + sortorder);
          items.insert(items.begin() + sortorder, itemid);
        }
        break;
      }
    }
}
    
  

void kmeal::listitem(
  uint64_t     book_id,
  uint64_t     item_id,
  uint64_t     section_id,
  char         list_type,
  float        list_price,
  float        min_price,
  uint64_t     quantity,
  uint32_t     duration, // event duration
  uint32_t expires,    
  uint64_t     sliding_rate,
  uint64_t     status,
  vector<listing_sides>   sides,
  bool         isactive
) {
  
    //auth the owner of the item
    auto _item = items.find(item_id);
    eosio_assert(_item != items.end(), "restaurant owner not found");
    require_auth(_item->owner.value);
    
    auto _book = books.find(book_id);
    eosio_assert(_book != books.end(), "book not found");
    require_auth(_book->owner.value);

    auto sections = _book->sections;
    bool sectionfound = false;
    bool itemfound = false;
    for (int r = 0; r < sections.size(); r++) {
    if(sections[r].section_id == section_id) {
      // todo
      // check if the item is added to the section
      // sectionfound = true;
      // auto item = eosio::find(sections[r].items,item_id);
    }
    }
    // get the listings and update
    auto _listings = listings.find(_item->owner.value);
    if( _listings == listings.end() ) {
      listings.emplace(_self, [&]( auto& s ) {
          s.listing_id = listings.available_primary_key();
          s.book_id = book_id;
          s.item_id = item_id;
          s.section_id = section_id;
          s.list_type = list_type;
          s.list_price = list_price;
          s.min_price = min_price;
          s.quantity = quantity;
          s.duration = duration;
          s.expires = time_point_sec(now()) +  expires;
          s.sliding_rate = sliding_rate;
          s.status = status;
          s.sides = sides;
          s.isactive =1;
      });
    } 
}

void kmeal::placeorder( uint64_t  order_id, name  buyer, name seller,  
      uint64_t order_status, 
      string  instructions,
      vector<orderdetail> detail) {
      
      eosio_assert(is_account(buyer), "buyer account does not exist");
      eosio_assert(is_account(seller), "seller account does not exist");
      
      require_auth(buyer);
      // check if restaurant_id exists
      auto _restaurant = restaurants.find(seller.value);
      eosio_assert(_restaurant != restaurants.end(), "restaurant not found");
      eosio_assert(_restaurant->owner != buyer, "cannot buy from your restaurant");
      
      // // check the length of order detail
      eosio_assert(detail.size() != 0, "items not exists");
      
                    
      for (auto &det : detail) // access by reference to avoid copying
      {   
           auto listing_id = det.listing_id;
          auto _listing = listings.find(listing_id);
          eosio_assert(_listing != listings.end(), "listing not found"); 
          eosio_assert(_listing->owner != _restaurant->owner, "listing not found from the restaurant");

          // check the listing type 
          // if dynamic check and the deal still exists or expired
          if (_listing->list_type == 'D') {
            eosio_assert(_listing->expires > time_point_sec(now()), "The deal is expired"); 
            uint32_t quantity =0;
            
            auto orderdx = orderdetails.get_index<"bylistingid"_n>();
            // todo
            auto iterator = orderdx.find(listing_id);
            if (iterator != orderdx.end()) {
              // for(iterator it = orderdx.begin(); it != orderdx.end(); ++it) {
              //   quantity = quantity + it.quantity;
              // }
              // for(auto& orderdetail : iterator ) {
              //    
              // }
            }
            //eosio_assert(quantity > _listing->quantity, "deal 100% claimed");
            eosio_assert(_listing->quantity >2, "cannot order more than 2");
          }
          else {
            // check for restaurant timing.. check if you could time the order
            
          }
      } 
  }
      

void kmeal::opendeposit(name owner)
{
    require_auth(owner);
    auto deposits_itr = deposits.find(owner.value);
    if(deposits_itr == deposits.end())
    {
        deposits.emplace(owner, [&](auto &row) {
            row.owner = owner;
            row.balance = asset(0, DEFAULT_SYMBOL);
        });
    }
}

void kmeal::closedeposit(name owner)
{
    eosio_assert(has_auth(owner) || has_auth(_self), "You do not have authority to close this deposit");
    deposit d = deposits.get(owner.value, "User does not have a deposit opened");
    if(d.balance.amount > 0)
    {
        transfer_kmeal(_self, owner, d.balance, "KMEAL deposit refund");
    }
    deposits.erase(deposits.find(owner.value));
}

void kmeal::depositkmeal(name from, name to, asset quantity, std::string memo)
{
    // In case the tokens are from us, or not to us, do nothing
    if(from == _self || from == KMEAL_ACCOUNT || to != _self)
        return;
    // This should never happen as we ensured transfer action belongs to "infinicoinio" account
    eosio_assert(quantity.symbol == DEFAULT_SYMBOL, "The symbol does not match");
    eosio_assert(quantity.is_valid(), "The quantity is not valid");
    eosio_assert(quantity.amount > 0, "The amount must be positive");
    auto deposits_itr = deposits.find(from.value);
    eosio_assert(deposits_itr != deposits.end(), "User does not have a deposit opened");

    deposits.modify(deposits_itr, same_payer, [&](auto &row){
        row.balance += quantity;
    });
}



// This function requires giving the active permission to the eosio.code permission
// cleos set account permission infiniverse1 active '{"threshold": 1,"keys": [{"key": "ACTIVE PUBKEY","weight": 1}],"accounts": [{"permission":{"actor":"infiniverse1","permission":"eosio.code"},"weight":1}]}' owner -p infiniverse1@owner
void kmeal::transfer_kmeal(name from, name to, asset quantity, std::string memo)
{
    action{
        permission_level{_self, "active"_n},
        KMEAL_ACCOUNT,
        "transfer"_n,
        std::tuple<name, name, asset, std::string>{from, to, quantity, memo}
    }.send();
}


extern "C" {
void apply(uint64_t receiver, uint64_t code, uint64_t action) {
    auto self = receiver;

    if( code == self ){ 
      switch(action) {
            EOSIO_DISPATCH_HELPER( kmeal,  
            (setuprest)
            (createbook)
            (opendeposit)
            (closedeposit)
            (depositkmeal)
            (addsections)
            (addtosection)
            (listitem)
            (createitem)
            (setsecorder)
            (delsec)
            (delrest)
            (placeorder)
            (cleartables))
      }
    }
    else {
        if(code == name("eosio.token").value && action == name("transfer").value){
            execute_action(name(receiver), name(code), &kmeal::depositkmeal);
        }
    }
}
};