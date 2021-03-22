#include <string>
#include <gtest/gtest.h>

#include "utils/string_utils.h"

using namespace std;

TEST(UtilsTest, gbd_utf8) {
    string utf8 = "566f10717fe66b6df974c926fbde899c4@@@纸上苍生而已";
    string gbk;
    utils::utf82gbk(utf8, gbk);

    string utf8FromGbk;
    utils::gbk2utf8(gbk, utf8FromGbk);

    EXPECT_EQ(utf8, utf8FromGbk);
}

TEST(UtilsTest, url_encode_decode) {
    string urlBody = "\"纵使文章惊海内\" AND entity_type = \"诗词\"";
    string urlEncoded = utils::urlEncode(urlBody);
    string urlDecoded = utils::urlDecode(urlEncoded);

    EXPECT_EQ(urlEncoded, "%22%E7%BA%B5%E4%BD%BF%E6%96%87%E7%AB%A0%E6%83%8A%E6%B5%B7%E5%86%85%22+AND+entity_type+%3D+%22%E8%AF%97%E8%AF%8D%22");
    EXPECT_EQ(urlBody, urlDecoded);
}

// TEST(UtilsTest, sign32) {
//     string s = "纵使文章惊海内";
//     termID32_t h32 = utils::hash32(s);
//     termID32_t s32 = utils::FNV_1a(s);
//     EXPECT_EQ(h32, s32);
// }