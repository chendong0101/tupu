#include <list>
#include <thread>
#include <signal.h>
#include <gtest/gtest.h>
#include "workflow/WFTaskFactory.h"
#include "workflow/WFOperator.h"
#include "workflow/HttpUtil.h"

#include "controller/http_service.h"
#include "message/entity.pb.h"
#include "utils/string_utils.h"

using namespace std;

class InterfaceTest : public testing::Test
{
};
/*
protected:
    static void SetUpTestSuite()
    {
        std::thread httpServerThread([]() {
            HttpService httpService;
            httpService.start();
        });
        httpServerThread.detach();
        usleep(100 * 1000);
    }
    static void TearDownTestSuite() {
        wait_group.done();
    }
};

TEST_F(InterfaceTest, get_by_doc_id)
{
	std::mutex mutex;
	std::condition_variable cond;
	bool done = false;
    string entityId = "3441984bd27cd18a4cde398a9d3fcd71";
	auto *task = WFTaskFactory::create_http_task("http://127.0.0.1:8888/tupu/doc?docId=" + entityId, 
                            0, 2, [&mutex, &cond, &done, entityId](WFHttpTask *task) {
		auto state = task->get_state();

		EXPECT_EQ(state, WFT_STATE_SUCCESS);
		if (state == WFT_STATE_SUCCESS)
		{
			auto code = atoi(task->get_resp()->get_status_code());
            EXPECT_EQ(200, HttpStatusOK);
            const void *body;
            size_t body_len;

            task->get_resp()->get_parsed_body(&body, &body_len);
            std::string value((char *)body, body_len);
            proto::SearchResult *sr = new proto::SearchResult();
            if (sr->ParseFromString(value))
            {
                EXPECT_EQ(sr->entity_contents_size(), 1);
                proto::EntityContent ec = sr->entity_contents()[0];
                EXPECT_EQ(ec.entity_id(), entityId);

                string v2;
                auto paires = ec.pairs();
                for_each(paires.begin(), paires.end(), [&v2](const proto::Pair &pair) {
                    if (pair.key() == "下一句") {
                        v2 = pair.value();
                    }
                });

                EXPECT_EQ(v2, "66f10717fe66b6df974c926fbde899c4@@@纸上苍生而已");
            } else {
                ASSERT_TRUE(false);
            }
        }

        mutex.lock();
		done = true;
		mutex.unlock();
		cond.notify_one();
	});
	task->start();

	std::unique_lock<std::mutex> lock(mutex);
	while (!done)
		cond.wait(lock);

	lock.unlock();
}

TEST_F(InterfaceTest, search_by_sql)
{
	std::mutex mutex;
	std::condition_variable cond;
	bool done = false;
    string sql = "SELECT \"所属诗词\" WHERE 内容 = \"纵使文章惊海内\" AND entity_type = \"诗词\" AND 下一句 = \"纸上苍生而已\" ORDER BY \"点击权重\" LIMIT 2";
    string url = "http://127.0.0.1:8888/tupu/sql?sql=" + utils::urlEncode(sql);
	auto *task = WFTaskFactory::create_http_task(url, 0, 2, [&mutex, &cond, &done](WFHttpTask *task) {
		auto state = task->get_state();

		EXPECT_EQ(state, WFT_STATE_SUCCESS);
		if (state == WFT_STATE_SUCCESS)
		{
			auto code = atoi(task->get_resp()->get_status_code());
            EXPECT_EQ(200, HttpStatusOK);
            const void *body;
            size_t body_len;

            task->get_resp()->get_parsed_body(&body, &body_len);
            std::string value((char *)body, body_len);
            proto::SearchResult *sr = new proto::SearchResult();
            if (sr->ParseFromString(value))
            {
                EXPECT_GT(sr->entity_contents_size(), 0);
                proto::EntityContent ec = sr->entity_contents()[0];
                auto paires = ec.pairs();
                string v1, v2;
                string k1, k2;
                utils::utf82gbk("内容", k1);
                utils::utf82gbk("下一句", k2);
                for_each(paires.begin(), paires.end(), [&v1, &v2, &k1, &k2](const proto::Pair &pair) {
                    if (pair.key() == k1) {
                        utils::gbk2utf8(pair.value(), v1);
                    } 
                    if (pair.key() == k2) {
                        utils::gbk2utf8(pair.value(), v2);
                    }
                });

                EXPECT_EQ(v1, "纵使文章惊海内");
                EXPECT_EQ(v2, "66f10717fe66b6df974c926fbde899c4@@@纸上苍生而已");
            } else {
                ASSERT_TRUE(false);
            }
		}

		mutex.lock();
		done = true;
		mutex.unlock();
		cond.notify_one();
	});
	task->start();

	std::unique_lock<std::mutex> lock(mutex);
	while (!done)
		cond.wait(lock);

	lock.unlock();
}
*/