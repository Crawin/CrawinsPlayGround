#pragma once
#include "LF_SkipList.h"
struct DB_DATA {
	unsigned long long userID;
	unsigned long long guildID;
	double START_TIME;
	double END_TIME;
	DB_DATA() {}
	DB_DATA(const unsigned long long& user_id, const unsigned long long& guild_id, const double& begin) : userID(user_id),guildID(guild_id), START_TIME(begin), END_TIME(0) {}
	void Save(nlohmann::ordered_json& j) {
		// 서버 리스트가 없으면 생성
		if (j.contains("SERVERS") == false) {
			j["SERVERS"] = nlohmann::ordered_json::array();
		}

		// 서버 찾기 또는 생성
		auto itServer = std::find_if(j["SERVERS"].begin(), j["SERVERS"].end(), [&](auto& s) {
			return s.contains("SERVERID") && s["SERVERID"] == guildID;
			});

		if (itServer == j["SERVERS"].end()) {
			j["SERVERS"].push_back({
				{ "SERVERID", guildID },
				{ "LOGS", nlohmann::ordered_json::array() }
				});
			itServer = std::prev(j["SERVERS"].end()); // 다시 iterator 얻기
		}

		// 유저 로그 찾기 또는 생성
		auto& logs = (*itServer)["LOGS"];
		auto itLog = std::find_if(logs.begin(), logs.end(), [&](auto& l) {
			return l.contains("ID") && l["ID"] == userID;
			});

		if (itLog == logs.end()) {
			logs.push_back({
				{ "ID", userID },
				{ "WORKINGTIME", nlohmann::ordered_json::object() }
				});
			itLog = std::prev(logs.end());
		}

		auto& log = *itLog;
		time_t startTimeSec = static_cast<time_t>(START_TIME);
		time_t endTimeSec = static_cast<time_t>(END_TIME);

		std::tm timeinfo{};
#ifdef _WIN32
		localtime_s(&timeinfo, &startTimeSec);
#else
		localtime_r(&timeSec, &timeinfo);
#endif

		std::string start_year = std::to_string(timeinfo.tm_year + 1900);
		std::string start_month = std::to_string(timeinfo.tm_mon + 1);
		std::string start_date = std::to_string(timeinfo.tm_mday);

		if (log["WORKINGTIME"].find(start_year) == log["WORKINGTIME"].end()) {
			log["WORKINGTIME"][start_year] = nlohmann::ordered_json::object();
		}
		if (log["WORKINGTIME"][start_year].find(start_month) == log["WORKINGTIME"][start_year].end()) {
			log["WORKINGTIME"][start_year][start_month] = nlohmann::json::object();
		}
		if (log["WORKINGTIME"][start_year][start_month].find(start_date) == log["WORKINGTIME"][start_year][start_month].end()) {
			log["WORKINGTIME"][start_year][start_month][start_date] = nlohmann::json::object();
		}

		time_t deltaSec = static_cast<time_t>(END_TIME - START_TIME);

		if (log["WORKINGTIME"][start_year][start_month][start_date].contains("TOTAL")) {
			deltaSec += std::stoll(log["WORKINGTIME"][start_year][start_month][start_date]["TOTAL"].get<std::string>());
		}
		log["WORKINGTIME"][start_year][start_month][start_date]["TOTAL"] = std::to_string(deltaSec);

		// 로그 추가
		char startBuf[9], endBuf[9];
		std::tm endTm{};

#ifdef _WIN32
		localtime_s(&endTm, &endTimeSec);
#else
		localtime_r(&endTimeSec, &endTm);
#endif

		std::strftime(startBuf, sizeof(startBuf), "%H:%M:%S", &timeinfo);
		std::strftime(endBuf, sizeof(endBuf), "%H:%M:%S", &endTm);

		log["WORKINGTIME"][start_year][start_month][start_date]["LOG"].push_back({
			{"START", startBuf},
			{"END", endBuf}
			});
	}
	std::string deltaTime() {
		return std::to_string(END_TIME - START_TIME);
	}
};

struct set {
	std::set<dpp::snowflake> s;
	std::mutex l;

	set& operator=(const set& other) {
		if (this != &other) {
			s = other.s;
		}
		return *this;
	}

	void emplace(const unsigned long long& id) {
		l.lock();
		s.emplace(id);
		l.unlock();
	}

	void Save(nlohmann::ordered_json& j, const std::string Key) {
		l.lock();
		for (const auto& i : s) {
			j[Key].emplace_back(std::to_string(i));
		}
		l.unlock();
	}
	
};

struct Whitelist {	// 해당 서버에서 추적중인 채널과 유저들
	double lastTime;
	set channelID;
	set userID;
	Whitelist():lastTime(0) {};

	Whitelist(const double& usedTime, const unsigned long long& ch_id, const unsigned long long& usr_id) :lastTime(usedTime) {
		if (ch_id) {
			channelID.l.lock();
			channelID.s.emplace(ch_id);
			channelID.l.unlock();
		}
		if (usr_id) {
			userID.l.lock();
			userID.s.emplace(usr_id);
			userID.l.unlock();
		}
	}
};

class Bot
{
private:
	const std::string m_strBottoken = "";
	dpp::cluster* m_pCluster;
	std::shared_mutex m_sm_wldb;
	crawin::LF_hash_skiplist<dpp::snowflake,DB_DATA> m_tracking;
	crawin::LF_hash_skiplist<dpp::snowflake, Whitelist> m_whitelist;
	crawin::LF_hash_skiplist<dpp::snowflake,DB_DATA> m_DB;

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
	void test(int n);
	void test_func(const int& start);
};
