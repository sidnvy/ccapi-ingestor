#include "ccapi_cpp/ccapi_session.h"
#include "clickhouse/client.h"
#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <string>
#include <thread> // Include the missing header for std::this_thread
#include <vector>

// Function to convert CCAPI DateTime to ClickHouse DateTime64
int64_t toClickHouseDateTime(const ccapi::DateTime &dt) {
  auto epoch = dt.getEpochTime();
  auto ns = dt.getNanoseconds();
  return epoch * 1000000000 + ns;
}

void processEvents(const std::vector<ccapi::Event> &events,
                   clickhouse::Client &client) {
  // Prepare data for ClickHouse
  std::vector<int64_t> timestamps;
  std::vector<std::string> symbols;
  std::vector<std::string> exchanges;
  std::vector<double> bid_prices;
  std::vector<double> ask_prices;
  std::vector<double> bid_sizes;
  std::vector<double> ask_sizes;

  for (const auto &event : events) {
    if (event.getType() == ccapi::Event::Type::SUBSCRIPTION_DATA) {
      for (const auto &message : event.getMessageList()) {
        const std::string &correlation_id = message.getCorrelationIdList()[0];
        auto delim_pos = correlation_id.find(',');
        std::string exchange = correlation_id.substr(0, delim_pos);
        std::string symbol = correlation_id.substr(delim_pos + 1);

        auto timestamp_start = toClickHouseDateTime(message.getTime());
        double bid_price = message.getElement("BID_PRICE").getValueDouble();
        double ask_price = message.getElement("ASK_PRICE").getValueDouble();
        double bid_size = message.getElement("BID_SIZE").getValueDouble();
        double ask_size = message.getElement("ASK_SIZE").getValueDouble();

        timestamps.push_back(timestamp_start);
        symbols.push_back(symbol);
        exchanges.push_back(exchange);
        bid_prices.push_back(bid_price);
        ask_prices.push_back(ask_price);
        bid_sizes.push_back(bid_size);
        ask_sizes.push_back(ask_size);
      }
    }
  }

  // Insert data into ClickHouse
  clickhouse::Block block;

  block.AppendColumn("timestamp", std::move(timestamps));
  block.AppendColumn("symbol", std::move(symbols));
  block.AppendColumn("exchanges", std::move(exchanges));
  block.AppendColumn("bid_price", std::move(bid_prices));
  block.AppendColumn("ask_price", std::move(ask_prices));
  block.AppendColumn("bid_size", std::move(bid_sizes));
  block.AppendColumn("ask_size", std::move(ask_sizes));

  client.Insert("quote_ticks", block);
}

int main() {
  // Initialize CCAPI session and set up subscriptions
  ccapi::SessionOptions session_options;
  ccapi::SessionConfigs session_configs;
  ccapi::Session session(session_options, session_configs);

  std::vector<Subscription> subscriptionList;
  ccapi::Subscription subscription;
  std::vector<std::string> exchanges = {"binance-usds-futures"};
  std::vector<std::string> symbols = {"ethusdt", "btcusdt", "bnbusdt"};
  for (const auto &exchange : exchanges) {
    for (const auto &symbol : symbols) {
      std::string correlation_id = exchange + "," + symbol;
      subscriptionList.emplace_back(exchange, symbol, "MARKET_DEPTH",
                                    "MARKET_DEPTH_MAX=1", correlation_id);
    }
  }
  session.subscribe(subscriptionList);

  // Connect to ClickHouse and create the table schema
  clickhouse::ClientOptions client_options("localhost:9000", "default",
                                           "default", "default");
  clickhouse::Client client(client_options);

  std::string table_schema = R"(
      CREATE TABLE IF NOT EXISTS quote_ticks
      (
          timestamp DateTime64 CODEC(Delta, ZSTD(1)),
          symbol LowCardinality(String) CODEC(ZSTD(1)),
          exchanges LowCardinality(String) CODEC(ZSTD(1)),
          bid_price Float64 CODEC(Delta, ZSTD(1)),
          ask_price Float64 CODEC(Delta, ZSTD(1)),
          bid_size Float64 CODEC(Delta, ZSTD(1)),
          ask_size Float64 CODEC(Delta, ZSTD(1))
      )
      ENGINE = MergeTree()
      ORDER BY timestamp;
  )";

  client.Execute(table_schema);

  // Run the session
  session.start();
  while (true) {
    std::this_thread::sleep_for(std::chrono::seconds(10));
    std::vector<Event> eventList = session.getEventQueue().purge();
    for (const auto &event : eventList) {
      processEvents(const int &events, int &client)
    }
  }
  session.stop();

  // Wait for the user to press Enter to stop the session
  std::cout << "Press Enter to stop the session..." << std::endl;
  std::cin.get();

  // Stop the session
  session.stop();

  return 0;
}
