#pragma once
#include "LF_SkipList.h"
struct USER_TIME {
	double START_TIME;
	double END_TIME;
	USER_TIME(const double& begin) :START_TIME(begin), END_TIME(0) {}
};

struct Whitelist {
	double lastTime;
	std::set<dpp::snowflake> channelID;
	std::set<dpp::snowflake> userID;
	Whitelist():lastTime(0) {};
	Whitelist(const double& usedTime, const unsigned long long& ch_id, const unsigned long long& usr_id) :lastTime(usedTime) {
		if (ch_id) {
			channelID.emplace(ch_id);
		}
		if (usr_id) {
			userID.emplace(usr_id);
		}
	}
};

struct DB_DATA {
	unsigned long long userID;
	double START_TIME;
	double END_TIME;
	//DB_DATA(const unsigned long long& userid, const double& epoch_start, const double& epoch_end) :userID(userid) {
	//	START_TIME = dpp::utility::timestamp(epoch_start, dpp::utility::tf_long_time);
	//	END_TIME = dpp::utility::timestamp(epoch_end, dpp::utility::tf_long_time);
	//}
	DB_DATA(const std::unordered_map<dpp::snowflake, USER_TIME>::iterator& it): userID(it->first), START_TIME(it->second.START_TIME), END_TIME(it->second.END_TIME) {}

	std::string convertTime(const long long& TIME) {
		return std::to_string((TIME / 3600 + 9) % 24) + " : " + std::to_string((TIME % 3600) / 60) + " : " + std::to_string((TIME % 60));
	}
};

class Bot
{
private:
	const std::string m_strBottoken = "";
	dpp::cluster* m_pCluster;
	
	// userID, startTime, endTime
	std::unordered_map<dpp::snowflake, USER_TIME> m_umTracking;
	std::mutex m_m_tracking;

	// serverID, lastTime. channelID, userID
	std::unordered_map<dpp::snowflake, Whitelist> m_umWhitelist;
	std::shared_mutex m_sm_wldb;
	std::mutex m_m_wl;

	//crawin::LF_skiplist< dpp::snowflake, Whitelist> m_lfslWhitelist;
	
	//crawin::LF_unordered_map<dpp::snowflake, Whitelist> m_lfum;
	crawin::LF_hash_skiplist<dpp::snowflake, Whitelist> m_lfhsl;

	// userID, startTime, endTime
	std::queue<DB_DATA> m_qDBdata;

	std::mutex m_dbmutex;

public:
	Bot();
	~Bot();
	void declare_commands();
	void define_commands();
	void Run();
	void save_db();
	void save_whitelist();
	bool find_whitelist_in_DB(const unsigned long long& guild_id);
	void cleanup_Memory();
	void test();
	void test_func(const int& start);
};

