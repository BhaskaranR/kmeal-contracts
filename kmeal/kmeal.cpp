#include "kmeal.hpp"

using namespace eosio;
using namespace std;

const symbol kmeal_symbol = symbol("KMEAL", 4);
const name kmeal_account = "kmealcoinio1"_n;

void kmeal::cleartables()
{
  require_auth(_self);
  cleanTable<sections_table>();
  cleanTable<books_table>();
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
                      const string timeofoperation)
{
  require_auth(account);
  eosio_assert(is_account(account), "account does not exist");
  eosio_assert(name.length() > 0, "name cannot be empty");
  eosio_assert(phone.length() > 0, "phone cannot be empty");
  eosio_assert(address.length() > 0, "address cannot be empty");
  eosio_assert(address2.length() > 0, "address cannot be empty");
  
  //todos other validation
  //assign permission for owner to onboard..
  auto accsetter = [&](auto &s) {
    s.owner = account;
    s.is_active = 1;
  };

  auto setter = [&](auto &s) {
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
  if (_restaurant == restaurants.end())
  {
    auto _account = accounts.find(account.value);
    eosio_assert(_account == accounts.end(), "already signed up as customer");
    auto itr = accounts.find(account.value);
    
    accounts.emplace(_self, [&](auto &a) {
      a.owner = account;
      a.balance = asset{0, kmeal_symbol};
    });
    restaurants.emplace(account, setter);
  }
  else
  {
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
  restaurants.modify(*_restaurant, account, [&](auto &item) {
    item.is_active = 0;
  });
}

void kmeal::createbook(const name account, const string bookname)
{
  require_auth(account);

  auto iter = restaurants.find(account.value);
  eosio_assert(iter != restaurants.end(), "restaurant owner not found");
  auto _books = books.find(account.value);
  //todo
  //check if bookname already exists..
  if (_books == books.end())
  {
    books.emplace(_self, [&](auto &s) {
      s.book_id = books.available_primary_key();
      s.owner = account;
      s.book_name = bookname;
      s.sections = vector<uint64_t>();
    });
  }
}

void kmeal::addsections(uint64_t bookid, std::string sectionname)
{
  auto iter = books.find(bookid);
  eosio_assert(iter != books.end(), "bookid not found");
  require_auth(iter->owner);
  
  auto sectionid = sections.available_primary_key();
  
  auto setter = [&](auto &sec) {
    sec.section_id = sectionid;
    sec.section_name = sectionname;
    sec.items = vector<uint64_t>();
  };
  
  sections.emplace(_self, setter);
  books.modify(iter, _self, [&](auto &p) {
    p.sections.push_back (sectionid); 
  });
  
}

void kmeal::setsecorder(uint64_t bookid, uint64_t sectionid, uint64_t sortorder)
{
  auto iter = books.find(bookid);
  eosio_assert(iter != books.end(), "bookid not found");
  require_auth(iter->owner);
  
  vector<uint64_t> sec = iter->sections;
  eosio_assert(sec.cbegin() != sec.cend(), "no section values yet");
  auto length = sec.size();
  eosio_assert(sortorder < length, "wrong sort order");
  
  std::vector<uint64_t>::iterator it = std::find(sec.begin(), sec.end(), sectionid);
  int index = std::distance(sec.begin(), it);
  sec.erase(sec.begin() + index);
  sec.insert(sec.begin() + sortorder, sectionid);
  
   books.modify(iter, _self, [&](auto &p) {
    p.sections = sec;
  });
 
  
}

void kmeal::delsec(uint64_t bookid, uint64_t secid)
{
  auto iter = books.find(bookid);
  eosio_assert(iter != books.end(), "bookid not found");
  require_auth(iter->owner);
  auto secItr = sections.find(secid);
  eosio_assert(secItr != sections.end(), "section does not exist.");
  //TODO check for exist order for the current section and then delete.
  vector<uint64_t> sec = iter->sections;
  std::vector<uint64_t>::iterator it = std::find(sec.begin(), sec.end(), secid);
  int index = std::distance(sec.begin(), it);
  sec.erase(sec.begin() + index);
  books.modify(iter, _self, [&](auto &p) {
    p.sections = sec;
  });
  sections.erase(secItr);
}

void kmeal::createitem(name account,
                      string itemname,
                      string description,
                      string photo,
                      uint64_t spicy_level,
                      uint64_t vegetarian,
                      uint64_t cooking_time,
                      vector<string> types)
{
  //todo  validate other inputs
  auto iter = restaurants.find(account.value);
  eosio_assert(iter != restaurants.end(), "restaurant owner not found");
  require_auth(iter->owner);

  auto _items = items.find(account.value);
  //find itemid by secoundary indx which is the item namespace
  if (_items == items.end())
  {
    items.emplace(_self, [&](auto &s) {
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
    uint64_t itemid,
    string itemname,
    string description,
    string photo,
    uint64_t spicy_level,
    uint64_t vegetarian,
    uint64_t cooking_time,
    vector<string> types)
{

  auto _item = items.find(itemid);
  eosio_assert(_item != items.end(), "item could not be found");
  require_auth(_item->owner);

  items.modify(*_item, _self, [&](auto &s) {
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

void kmeal::addtosection(uint64_t bookid, uint64_t sectionid, uint64_t itemid, uint64_t sortorder)
{
  //auth the owner of bookid
  auto iter = books.find(bookid);
  eosio_assert(iter != books.end(), "bookid not found");
  require_auth(iter->owner);

  //auth the owner of itemid
  auto i_iter = items.find(itemid);
  eosio_assert(i_iter != items.end(), "itemid not found");
  require_auth(i_iter->owner);
  
  //loop through sections
  auto secItr = sections.find(sectionid);
  eosio_assert(secItr != sections.end(), "section does not exist.");
  
  vector<uint64_t> secitems = secItr->items;
  auto itemid_itr = std::find(secitems.begin(), secitems.end(), itemid);
  if (itemid_itr != secitems.end()) {
    eosio::print("found");
    std::vector<uint64_t>::iterator it = std::find(secitems.begin(), secitems.end(), itemid);
    int index = std::distance(secitems.begin(), it);
    secitems.erase(secitems.begin() + index);
    secitems.insert(secitems.begin() + sortorder, itemid);
    
    sections.modify(secItr, _self, [&](auto &p) {
      p.items = secitems;
    });
  }
  else {
    eosio::print("not found");
    sections.modify(secItr, _self, [&](auto &p) {
      p.items.push_back(itemid);
    });
  }
}

void kmeal::removefromsection(uint64_t bookid, uint64_t sectionid, uint64_t itemid) {
  
}


void kmeal::listitem(
    uint64_t book_id,
    uint64_t item_id,
    uint64_t section_id,
    uint64_t list_type,
    float list_price,
    float min_price,
    uint64_t quantity,
    uint32_t expires,
    float sliding_rate,
    vector<listing_sides> sides)
{

  //auth the owner of the item
  auto _item = items.find(item_id);
  eosio_assert(_item != items.end(), "item not found");
  require_auth(_item->owner.value);

  auto _book = books.find(book_id);
  eosio_assert(_book != books.end(), "book not found");
  require_auth(_book->owner.value);

  auto secItr = sections.find(section_id);
  eosio_assert(secItr != sections.end(), "section does not exist.");
  
  auto itemid_itr = std::find(secItr->items.begin(), secItr->items.end(), item_id);
  eosio_assert(itemid_itr != secItr->items.end(), "item not added to the section");
 
  //TODO validation for the listings
  auto _listings = listings.find(_item->owner.value);
  
  if (_listings == listings.end())
  {
    listings.emplace(_self, [&](auto &s) {
      s.listing_id = listings.available_primary_key();
      s.book_id = book_id;
      s.item_id = item_id;
      s.section_id = section_id;
      s.list_type = list_type;
      s.list_price = list_price;
      if (list_type != REGULAR_LIST_TYPE_FLAG) {
        s.min_price = min_price;
        s.quantity = quantity;
        s.expires = time_point_sec(now()) + expires;
      }
      s.sliding_rate = sliding_rate;
      
      s.status = 1;
      s.sides = sides;
      s.isactive = 1;
    });
  }
}

void kmeal::placeorder(name buyer, name seller,
                      string instructions,
                      vector<orderdetail> detail)
{

  eosio_assert(is_account(buyer), "buyer account does not exist");
  eosio_assert(is_account(seller), "seller account does not exist");

  require_auth(buyer);
  // check if restaurant_id exists
  auto _restaurant = restaurants.find(seller.value);
  eosio_assert(_restaurant != restaurants.end(), "restaurant not found");
  eosio_assert(_restaurant->owner != buyer, "cannot buy from your restaurant");
  // // check the length of order detail
  eosio_assert(detail.size() != 0, "items not exists");

 
  vector<uint64_t> detail_ids;
  for (auto &det : detail)
  { // access by reference to avoid copying
    auto listing_id = det.listing_id;
    auto _listing = listings.find(listing_id);
    eosio_assert(_listing != listings.end(), "listing not found");
    eosio_assert(_listing->owner != _restaurant->owner, "listing not found from the restaurant");
    // check the listing type
    // if dynamic check and the order still exists or expired
    if (_listing->list_type == DYNAMIC_LIST_TYPE_FLAG)
    {
      eosio_assert(_listing->expires > time_point_sec(now()), "The order is expired");
      uint32_t quantity = 0;
      auto orderdx = orderdetails.get_index<"bylistingid"_n>();
      // todo
      auto iterator = orderdx.find(listing_id);
      if (iterator != orderdx.end())
      {
        // for(iterator it = orderdx.begin(); it != orderdx.end(); ++it) {
        //   quantity = quantity + it.quantity;
        // }
        // for(auto& orderdetail : iterator ) {
        //
        // }
      }
      //eosio_assert(quantity > _listing->quantity, "order 100% claimed");
      eosio_assert(_listing->quantity > 2, "cannot order more than 2");
    }
    else
    {
      // check for restaurant timing.. check if you could time the order
      eosio::print(_restaurant->timeofoperation);
    }
    //loop orderdetail and insert..
    auto key = orderdetails.available_primary_key();
    orderdetails.emplace(_self, [&](auto &s) {
      s.order_detail_id = key;
      s.listing_id = det.listing_id;
      s.quantity = det.quantity;
      s.ordered_price = det.ordered_price;
      s.final_price = det.final_price;
      s.instructions = det.instructions;
    });
    if (_listing->list_type == DYNAMIC_LIST_TYPE_FLAG)
    {
      uint64_t id = 0;
      auto size = transaction_size();
      char buf[size];
      uint32_t read = read_transaction(buf, size);
      eosio_assert(size == read, "read_transaction failed");
      capi_checksum256 h;
      sha256(buf, read, &h);
      for (int i = 0; i < 4; i++)
      {
        id <<= 8;
        id |= h.hash[i];
      }
      auto idx = orders.emplace(_self, [&](order &s) {
        s.order_id = id;
        s.buyer = buyer;
        s.seller = seller;
        s.order_type = DYNAMIC_ORDER_FLAG;
        s.flags |= BUYER_ORDERED_FLAG;
        s.expires = _listing->expires;
        s.instructions = instructions;
        s.detail.push_back(key);
      });
      //_notify(name("new"), "New order created", *idx);
    }
    else
    {
      detail_ids.push_back(key);
    }
  }
  if (detail_ids.size() > 1)
  {
    uint64_t id = 0;
      auto size = transaction_size();
      char buf[size];
      uint32_t read = read_transaction(buf, size);
      eosio_assert(size == read, "read_transaction failed");
      capi_checksum256 h;
      sha256(buf, read, &h);
      for (int i = 0; i < 4; i++)
      {
        id <<= 8;
        id |= h.hash[i];
      }
    auto idx = orders.emplace(_self, [&](order &s) {
      s.order_id = id;
      s.buyer = buyer;
      s.seller = seller;
      s.order_type = 'R';
      s.flags |= BUYER_ORDERED_FLAG;
      s.instructions = instructions;
      vector<uint64_t> detail(detail_ids);
      s.detail = detail;
    });
    //_notify(name("new"), "New order created", *idx);
  }
  require_recipient(buyer);
  require_recipient(seller);
  //clean_expiredorders
}

void kmeal::accept(name seller, uint64_t order_id)
{
  require_auth(seller);
  auto orderitr = orders.find(order_id);
  eosio_assert(orderitr != orders.end(), "Cannot find order_id");
  const order &d = *orderitr;
  auto flags = d.flags;

  eosio_assert(d.expires > time_point_sec(now()), "The order is expired");

  if (seller == d.seller)
  {
    eosio_assert((d.flags & SELLER_ACCEPTED_FLAG) == 0, "Seller has already accepted this order");
    flags |= SELLER_ACCEPTED_FLAG;
  }
  else
  {
    eosio_assert(false, "Order can only be accepted by seller");
  }

  if ((flags & BOTH_ACCEPTED_FLAG) == BOTH_ACCEPTED_FLAG)
  {
    orders.modify(*orderitr, seller, [&](auto &item) {
      item.flags = flags;
    });
    //_notify(name("accepted"), "order is fully accepted", d);
    require_recipient(d.seller);
    require_recipient(d.buyer);
  }
  else
  {
    orders.modify(*orderitr, seller, [&](auto &item) {
      item.flags = flags;
    });
  }

  //todo
  //_clean_expiredorders(order_id);
}

void kmeal::cancel(uint64_t order_id)
{
  auto orderitr = orders.find(order_id);
  eosio_assert(orderitr != orders.end(), "Cannot find order_id");
  const order &d = *orderitr;

  eosio_assert((d.flags & ORDER_ARBITRATION_FLAG) == 0, "The order is in arbitration");
  eosio_assert(d.expires > time_point_sec(now()), "The order is expired");

  if ((d.flags & ORDER_FUNDED_FLAG) == 0)
  {
    // not funded, so any of the parties can cancel the order
    eosio_assert(has_auth(d.buyer) || has_auth(d.seller),
                "Only seller or buyer can cancel the order");
  }
  else
  {
    eosio_assert((d.flags & ORDER_DELIVERED_FLAG) == 0, "The order is already delivered, cannot cancel");
    // funded, so only seller can cancel the order
    eosio_assert(has_auth(d.seller), "The order is funded, so only seller can cancel it");
    
    send_payment(d.buyer, d.total_price, string("order ") + to_string(d.order_id) + ": canceled by seller");
    //_notify(name("refunded"), "order canceled by seller, buyer got refunded", d);
  }

  //_notify(name("canceled"), "The order is canceled", d);
  orders.erase(orderitr);
  //_clean_expiredorders(order_id);
}

// Goods Received may be made before "delivered", but the ordered must be funded first
void kmeal::delivered(uint64_t order_id, string memo)
{
  auto orderitr = orders.find(order_id);
  eosio_assert(orderitr != orders.end(), "Cannot find order_id");
  const order &d = *orderitr;

  eosio_assert(d.expires > time_point_sec(now()), "The ordered is expired");
  eosio_assert((d.flags & ORDER_FUNDED_FLAG), "The ordered is not funded yet");
  eosio_assert((d.flags & ORDER_DELIVERED_FLAG) == 0, "The ordered is already marked as delivered");
  eosio_assert(has_auth(d.seller), "Only seller can mark a ordered as delivered");

  orders.modify(*orderitr, d.seller, [&](auto &item) {
    item.expires = time_point_sec(now()) + DELIVERED_ORDER_EXPIRES;
    item.flags |= ORDER_DELIVERED_FLAG;
    item.delivery_memo = memo;
  });

  //_notify(name("delivered"), "Order is marked as delivered", d);
  require_recipient(d.buyer);
  //_clean_expiredorders(order_id);
}

void kmeal::goodsrcvd(uint64_t order_id)
{
  auto orderitr = orders.find(order_id);
  eosio_assert(orderitr != orders.end(), "Cannot find order_id");
  const order &d = *orderitr;

  eosio_assert((d.flags & ORDER_FUNDED_FLAG), "The ordered is not funded yet");
  eosio_assert(has_auth(d.buyer), "Only buyer can sign-off Goods Received");

  send_payment(d.seller, d.total_price,
                string("Order ") + to_string(d.order_id) + ": goods received, order closed");

  //_notify(name("closed"), "Goods received, ordered closed", d);
  if (d.flags & ORDER_ARBITRATION_FLAG)
  {
    // require_recipient(d.arbiter);
  }
  orders.erase(orderitr);
  //_clean_expiredorders(order_id);
}

void kmeal::opendeposit(name owner)
{
  require_auth(owner);
  auto deposits_itr = deposits.find(owner.value);
  if (deposits_itr == deposits.end())
  {
    deposits.emplace(owner, [&](auto &row) {
      row.owner = owner;
      row.balance = asset(0, kmeal_symbol);
    });
  }
}

void kmeal::closedeposit(name owner)
{
  eosio_assert(has_auth(owner) || has_auth(_self), "You do not have authority to close this deposit");
  deposit d = deposits.get(owner.value, "User does not have a deposit opened");
  if (d.balance.amount > 0)
  {
    send_payment(owner, d.balance, "KMEAL deposit refund");
  }
  deposits.erase(deposits.find(owner.value));
}

void kmeal::transfer_handler(name from, name to, asset quantity, std::string memo)
{
  // In case the tokens are from us, or not to us, do nothing
  if (from == _self || from == kmeal_account || to != _self)
    return;

  // This should never happen as we ensured transfer action belongs to "kmealcoinio1" account
  eosio_assert(quantity.symbol == kmeal_symbol, "The symbol does not match");
  eosio_assert(quantity.is_valid(), "The quantity is not valid");
  eosio_assert(quantity.amount > 0, "The amount must be positive");

  eosio_assert(memo.length() > 0, "Memo must contain a valid order ID");

  uint64_t order_id = 0;
  for (int i = 0; i < memo.length(); i++)
  {
    char c = memo[i];
    eosio_assert('0' <= c && c <= '9', "Invalid character in symbol name. Expected only digits");
    order_id *= 10;
    order_id += (c - '0');
  }

  auto orderitr = orders.find(order_id);
  eosio_assert(orderitr != orders.end(), (string("Cannot find order ID: ") + to_string(order_id)).c_str());
  const order &d = *orderitr;

  eosio_assert(d.expires > time_point_sec(now()), "The order is expired");

  eosio_assert((d.flags & ORDER_FUNDED_FLAG) == 0, "The order is already funded");
  eosio_assert((d.flags & BOTH_ACCEPTED_FLAG) == BOTH_ACCEPTED_FLAG,
              "The order is not accepted yet by both parties");
  eosio_assert(from == d.buyer, "The order can only funded by buyer");
  
  // todo
  // const extended_asset payment(quantity, name{get_code()});
  // eosio_assert(payment == d.total_price,
  //             (string("Invalid amount or currency. Expected ") +
  //               d.total_price.quantity.to_string() + " via " + d.price.contract.to_string())
  //                 .c_str());
  
  orders.modify(*orderitr, _self, [&](auto &item) {
    item.funded = time_point_sec(now());
    //item.expires = item.funded + (item.days * DAY_SEC);
    item.flags |= ORDER_FUNDED_FLAG;
  });

  auto deposits_itr = deposits.find(from.value);
  eosio_assert(deposits_itr != deposits.end(), "User does not have a deposit opened");

  deposits.modify(deposits_itr, same_payer, [&](auto &row) {
    row.balance += quantity;
  });

  //_notify(name("funded"), "order is funded", d);
  require_recipient(d.seller);
  // _clean_expiredorders(order_id);
}

void kmeal::depositkmeal(name from, name to, asset quantity, std::string memo)
{
  // In case the tokens are from us, or not to us, do nothing
  if (from == _self || from == kmeal_account || to != _self)
    return;
  // This should never happen as we ensured transfer action belongs to "infinicoinio" account
  eosio_assert(quantity.symbol == kmeal_symbol, "The symbol does not match");
  eosio_assert(quantity.is_valid(), "The quantity is not valid");
  eosio_assert(quantity.amount > 0, "The amount must be positive");
  auto deposits_itr = deposits.find(from.value);
  eosio_assert(deposits_itr != deposits.end(), "User does not have a deposit opened");

  deposits.modify(deposits_itr, same_payer, [&](auto &row) {
    row.balance += quantity;
  });
}

// This function requires giving the active permission to the eosio.code permission
// cleos set account permission kmealadmin15 active '{"threshold": 1,"keys": [{"key": "ACTIVE PUBKEY","weight": 1}],"accounts": [{"permission":{"actor":"kmealadmin15","permission":"eosio.code"},"weight":1}]}' owner -p kmealadmin15@owner
void kmeal::send_payment(name to, asset quantity, std::string memo)
{
  action{
      permission_level{_self, name("active")},
      kmeal_account,
      name("transfer"),
      transfer{
          .from = _self, .to = to, .quantity = quantity, .memo = memo}
      }
      .send();
}

void kmeal::notify(name order_status, uint64_t order_id, string description, asset quantity,
                  name buyer, name seller, name arbiter, uint32_t days, string order_memo)
{
  require_auth(_self);
}

// leave a trace in history
// void _notify(name order_status, const string message, const order &d)
// {
//       action{
//           permission_level{_self, name("active")},
//           _self,
//           name("notify"),
//           order_notification_abi{
//               .order_status = order_status,
//               .message = message,
//               .order_id = d.order_id,
//               .tkcontract = d.price.contract,
//               .quantity = d.price.quantity,
//               .buyer = d.buyer,
//               .seller = d.seller,
//               .arbiter = d.arbiter,
//               .days = d.days,
//               .memo = d.delivery_memo}}
//           .send();
// }

// void kmeal::notify(name order_status, string message, uint64_t order_id, name created_by,
//                   string description, name tkcontract, asset &quantity,
//                   name buyer, name seller, name arbiter, uint32_t days, string delivery_memo)
// {
//   require_auth(_self);
// }

extern "C"
{
  void apply(uint64_t receiver, uint64_t code, uint64_t action)
  {
    auto self = receiver;

    if (code == self)
    {
      switch (action)
      {
        EOSIO_DISPATCH_HELPER(kmeal,
                              (setuprest)
                              
                              (createbook)(opendeposit)(closedeposit)(depositkmeal)(addsections)(addtosection)(listitem)(createitem)(setsecorder)(delsec)(delrest)(placeorder)(accept)(cleartables)
                              )
      }
    }
    else
    {
      if (code == kmeal_account.value && action == "transfer"_n.value)
      {
        execute_action(name(receiver), name(code), &kmeal::transfer_handler);
      }
    }
  }
};