#include "gtest/gtest.h"
#include <alibabacloud/credential_utils.hpp>
#include <cpprest/http_client.h>
#include <json/json.h>

#include <boost/date_time.hpp>
#include <list>

using namespace boost::gregorian;
using namespace boost::posix_time;

TEST(tests_utils, lowercase) { ASSERT_EQ("test", lowercase("TEST")); }

TEST(tests_utils, json_decode) {
  string json = R"({"foo":"bar"})";
  Json::Value result = json_decode(json);
  ASSERT_EQ("bar", result["foo"].asString());
}

TEST(tests_utils, hmacSha1) {
  string res = hmacSha1("test string", "secret");
  ASSERT_EQ("3Sa/3fEiwQVdTNWwVCJ3J+Hj7s8=", res);
}

TEST(tests_utils, url_encode) {
  ASSERT_EQ("%2F%3Ffoo%3Dbar%26a%3Db%26c%3Db%20f",
            url_encode("/?foo=bar&a=b&c=b f"));
}

TEST(tests_utils, httpQueryString) {
  map<string, string> query;
  query.insert(pair<string, string>("foo", "bar"));
  query.insert(pair<string, string>("encode", "a b/f*g!@ved"));
  query.insert(pair<string, string>("empty", ""));
  ASSERT_EQ("encode=a%20b%2Ff%2Ag%21%40ved&foo=bar", httpQueryString(query));
}

TEST(tests_utils, composeUrl) {
  map<string, string> params;
  params.insert(pair<string, string>("foo", "bar"));
  params.insert(pair<string, string>("value", "1"));
  params.insert(pair<string, string>("empty", ""));
  string url = composeUrl("example.com", params, "/pathname", "http", "80");
  ASSERT_EQ("http://example.com/pathname?foo=bar&value=1", url);

  url = composeUrl("example.com", params, "/pathname", "http", "8080");
  ASSERT_EQ("http://example.com:8080/pathname?foo=bar&value=1", url);
}

TEST(tests_utils, stringToSign) {
  map<string, string> query;
  query.insert(pair<string, string>("foo", "bar"));
  string strToSign = stringToSign(methods::GET, query);
  ASSERT_EQ("GET&%2F&foo%3Dbar", strToSign);
}

TEST(tests_utils, strtotime) {
  string datetime = "2020-08-28T05:39:40Z";
  ASSERT_EQ(1598593180, strtotime(datetime));
}
