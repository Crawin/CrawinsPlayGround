#pragma once
#include <atomic>
namespace crawin {
	/*{
		template<typename Key, typename Value>
		class LF_skiplist_fail
		{
		private:
			static constexpr int max_level = 5;
			class Node;

			class Stamped_Node {
				std::atomic_llong sptr;
				Node* get_ptr(bool& removed) {
					long long p = sptr;
					removed = (p & 1) == 1;
					return reinterpret_cast<Node*>(p & 0xFFFFFFFFFFFFFFFE);
				}
			};

			class Node {
			public:
				Key key;
				Value value;
				unsigned int level;
				Node* volatile next[max_level + 1];

				Node() {};
				Node(const Key& k, const Value& v, const unsigned int& lv) :key(k), value(v), level(lv) {
					for (int i = 0; i <= max_level; ++i) {
						next[i] = nullptr;
					}
				}
			};
			Node head, tail;

		public:
			LF_skiplist_fail() {
				head.key = 0;
				head.level = max_level;
				if (std::numeric_limits<Key>::max() > 0) {
					tail.key = std::numeric_limits<Key>::max();
				}
				else {
					tail.key = std::numeric_limits<unsigned long long>::max();
				}
				tail.level = max_level;
				for (int i = 0; i <= max_level; ++i) {
					head.next[i] = &tail;
				}
			}

			~LF_skiplist_fail() {
			}

			bool find(const Key& key, Node* preds[], Node* currs[]) {
				Node* prev = &head;
				Node* next = nullptr;
				for (int level = max_level; level >= 0; --level) {
					next = prev->next[level];

					if (next->key < key) {
						prev = next;
						++level;
						continue;
					}
					preds[level] = prev;
					currs[level] = next;
				}
				return next->key == key;
			}

			bool insert(const Key& key, const Value& value) {
				Node* n = new Node(key, value, rand() % (max_level + 1));
				Node* pred[max_level + 1]{ nullptr, };
				Node* curr[max_level + 1]{ nullptr, };
				while (true) {
					bool found = find(key, pred, curr);
					if (found) {
						delete n;
						return false;
					}
					for (int level = 0; level <= n->level; ++level) {
						n->next[level] = curr[level];
					}
					if (std::atomic_compare_exchange_strong(reinterpret_cast<volatile std::atomic_llong*>(&(pred[0]->next[0])), reinterpret_cast<long long*>(&curr[0]), reinterpret_cast<long long>(n)) == false) {
						continue;
					}
					for (int level = 1; level <= n->level; ++level) {
						while (true) {
							if (std::atomic_compare_exchange_strong(reinterpret_cast<volatile std::atomic_llong*>(&(pred[level]->next[level])), reinterpret_cast<long long*>(&curr[level]), reinterpret_cast<long long>(n))) {
								break;
							}
							find(key, pred, curr);
						}
					}
					return true;
				}
			}

			int getsize() {
				Node* curr = &head;
				int cnt = 0;
				while (curr->next[0] != &tail) {
					curr = curr->next[0];
					++cnt;
				}
				return cnt;
			}
		};

		// 2회차 구현 시작 atomic하게 메모리가 release 되지 않아 실패
		template<typename Key, typename Value>
		class LF_unordered_map_fail
		{
		private:
			static constexpr int base_bucket_size = 111;
			struct Node {
				Key key;
				Value value;
			};
			class stamped_Node_ptr {
				unsigned long long stptr;
			public:
				bool get_next(Node*& next) {
					next = stptr & 0xFFFFFFFFFFFFFFFE;
					return stptr & 1;
				}
			};
			class Using_Nodes {
			public:
				unsigned int hashed;	// 1: hashed 2: erasing
				std::atomic_uint bucket_fill;
				unsigned int bucket_size;
				std::atomic_uint ref_cnt;
				stamped_Node_ptr stamped_nodes;
				Node* nodes;
				Using_Nodes(const unsigned int& size) : hashed(0), bucket_fill(0), ref_cnt(0) {
					bucket_size = size;
					nodes = new Node[bucket_size];
				}
			};
			Using_Nodes* curr;
			Using_Nodes* next;
			std::hash<Key> hash_func;
		public:
			LF_unordered_map_fail() {
				curr = new Using_Nodes(base_bucket_size);
				next = new Using_Nodes(base_bucket_size * 2);
			}

			~LF_unordered_map_fail() {
				delete[] curr->nodes;
				delete curr;
				delete[] next->nodes;
				delete next;
			}

			// return true when filled more than half
			inline const bool check_load_factor(const Using_Nodes& un) const {
				return un.bucket_fill * 2 >= un.bucket_size;
			}

			void release_next() {
				Using_Nodes* next_nodes = next;
				if (next_nodes->ref_cnt == 0 && next_nodes->hashed == 1) {
					unsigned int one = 1;
					if (std::atomic_compare_exchange_strong(reinterpret_cast<std::atomic_uint*>(&next_nodes->hashed), &one, 2)) {





						int multiplier = (next_nodes->bucket_size / base_bucket_size) + 1;
						next_nodes->bucket_size = base_bucket_size * multiplier;
						Node* remove = next_nodes->nodes;
						next_nodes->nodes = new Node[next_nodes->bucket_size];

						delete[] remove;
						next_nodes->bucket_fill = 0;
						next_nodes->ref_cnt = 0;
						next_nodes->hashed = 0;
					}
				}
			}

			const bool find(const Key& key, Value& value) {
				unsigned int left = 0;

				//if (check_load_factor(*curr,left)) {		// curr bucket이 가득 차 가면
				//	int start_index = hash_func(key) % next->bucket_size;
				//	int index = start_index;
				//	do {
				//		if (next->nodes[index].key == key) {
				//			value = next->nodes[index].value;
				//			return true;
				//		}
				//		index = (index + 1) % next->bucket_size;
				//	} while (index != start_index);

				//	// next 에서 먼저 find 이후 없다면 curr까지 find
				//}
				// curr에서 찾아보자

				//release_next();

				Using_Nodes* curr_nodes = curr;
				++curr_nodes->ref_cnt;
				int start_index = hash_func(key) % curr_nodes->bucket_size;
				int index = start_index;
				do {
					if (curr_nodes->nodes[index].key == key) {
						value = curr_nodes->nodes[index].value;
						--curr_nodes->ref_cnt;
						return true;
					}
					index = (index + 1) % curr_nodes->bucket_size;
				} while (index != start_index);

				--curr_nodes->ref_cnt;
				return false;
			}

			void rehashing() {
				Using_Nodes* prev = curr;
				if (check_load_factor(*prev)) {
					if (std::atomic_compare_exchange_strong(reinterpret_cast<std::atomic_ullong*>(&curr), reinterpret_cast<unsigned long long*>(&prev), reinterpret_cast<long long>(next))) {
						prev->hashed = 1;
						next = prev;
					}
				}
			}

			bool insert(Key key, const Value& value) {
				// 3360 
				Value v;
				while (true) {
					if (find(key, v)) {	// 이미 키가 존재하면
						return false;
					}

					Using_Nodes* curr_nodes = curr;
					Using_Nodes* next_nodes = next;
					++curr_nodes->ref_cnt;
					++next_nodes->ref_cnt;
					if (check_load_factor(*curr_nodes)) {	// 가득 차 가기에 rehashing을 시작해야한다.
						rehashing();
						--curr_nodes->ref_cnt;
						--next_nodes->ref_cnt;
						continue;
					}
					int first_index = hash_func(key) % curr_nodes->bucket_size;
					int index = first_index;
					do {
						unsigned long long empty_key = curr_nodes->nodes[index].key;
						if (empty_key == 0) {	// 키가 비어있다.
							if (std::atomic_compare_exchange_strong(reinterpret_cast<std::atomic_ullong*>(&curr_nodes->nodes[index].key), &empty_key, static_cast<long long>(key))) { //내가 자리 차지에 성공했다.
								curr_nodes->nodes[index].value = value;
								++curr_nodes->bucket_fill;
								--curr_nodes->ref_cnt;

								// curr_nodes에는 끝 이제 next_nodes에 추가해주자
								if (curr_nodes != next_nodes && next_nodes->hashed == 0) {
									first_index = hash_func(key) % next_nodes->bucket_size;
									index = first_index;
									do {
										empty_key = next_nodes->nodes[index].key;
										if (empty_key == 0) {
											if (std::atomic_compare_exchange_strong(reinterpret_cast<std::atomic_ullong*>(&next_nodes->nodes[index].key), &empty_key, static_cast<long long>(key))) {
												next_nodes->nodes[index].value = value;
												break;
											}
										}
										index = (index + 1) % next_nodes->bucket_size;
									} while (index != first_index);
								}
								--next_nodes->ref_cnt;
								return true;
							}
						}
						index = (index + 1) % curr_nodes->bucket_size;	// 실패했으면 다음 자리로 이동
					} while (index != first_index);
					// 끝났다는것은 한바퀴 도는 동안 자리가 없었다는 것.
					// 다시 처음부터 insert 를 돌아보자.
					--curr_nodes->ref_cnt;
					--next_nodes->ref_cnt;
				}
			}
		};

		template<typename Key, typename Value>
		class LF_unordered_map_failed_two {
		private:
			struct Node {
				Key key;
				Value value;
				Node() {};
				Node(const Key& k, const Value& v, const unsigned int& lv) :key(k), value(v), level(lv) {
					for (int i = 0; i <= max_level; ++i) {
						next[i] = nullptr;
					}
				}
			};

			class stamped_node_ptr {
				unsigned long long stptr;
			public:
				stamped_node_ptr() {

				}
				bool get_next(Node*& next) {
					next = stptr & 0xFFFFFFFFFFFFFFFE;
					return stptr & 1;
				}
			};

			class skip_list {
			private:
				Node* n_head = new Node;

				stamped_node_ptr head, tail;

			};

			unsigned int bucket_size;
			skip_list* buckets;
			std::hash<Key> hash_func;
		public:
			LF_unordered_map_failed_two() : bucket_size(111) {
				buckets = new skip_list[bucket_size];
			}
			bool find() {

			}
			void insert(const Key& key, const Value& value) {
				hash_func(key);
			}
			void remove() {

			}
		};
	}
	*/

	template<typename Key, typename Value>
	class LF_hash_skiplist {
	private:
		std::atomic_int n_threads = 1;
		static constexpr char max_level = 5;

		class Node;

		class Stamped_Node {
		private:
			std::atomic_ullong stptr;
		public:
			void set_ptr(Node* ptr) {
				stptr = reinterpret_cast<unsigned long long>(ptr);
			}

			Node* get_ptr(bool& removed) {
				long long p = stptr;
				removed = (p & 1) == 1;
				return reinterpret_cast<Node*>(stptr & 0xFFFFFFFFFFFFFFFE);
			}

			bool CAS(Node* old_p, Node* new_p, bool old_m, bool new_m)
			{
				unsigned long long old_v = reinterpret_cast<unsigned long long>(old_p);
				if (true == old_m) old_v = old_v | 1;
				else
					old_v = old_v & 0xFFFFFFFFFFFFFFFE;
				unsigned long long new_v = reinterpret_cast<unsigned long long>(new_p);
				if (true == new_m) new_v = new_v | 1;
				else
					new_v = new_v & 0xFFFFFFFFFFFFFFFE;
				return std::atomic_compare_exchange_strong(&stptr, &old_v, new_v);
			}
		};

		class Node {
		public:
			Key k;
			Value v;
			int level;
			Stamped_Node next[max_level + 1];
			unsigned long long ebr_number;
			Node() {}
			Node(const Key& key, const Value& value, const int& top) :k(key), v(value), level(top), ebr_number(0) {
				for (int i = 0; i <= max_level; ++i) {
					next[i].set_ptr(nullptr);
				}
			}
		};

		class EBR {
			std::atomic_ullong epoch_counter;
			std::atomic_ullong* epoch_array;
			inline static char max_threads = std::thread::hardware_concurrency();
		public:
			EBR() :epoch_counter(1) {
				epoch_array = new std::atomic_ullong[max_threads * 16];
				//std::cout << "EBR 생성자"<< std::thread::hardware_concurrency()<<"개 생성\n";
			}
			~EBR() {
				//std::cout << "EBR 소멸자\n";
				delete[] epoch_array;
				epoch_counter = 1;
			}
			void start_epoch() {
				unsigned long long epoch = epoch_counter++;
				epoch_array[(m_thread_id - 1) * 16] = epoch;
			}

			void end_epoch() {
				epoch_array[(m_thread_id - 1) * 16] = 0;
			}

			Node* get_node(Key key, Value value) {
				int lv = 0;
				for (int i = 0; i < max_level; ++i) {
					if (rand() % 2 == 0) break;
					lv++;
				}

				if(m_free_queue.node_queue.empty())
					return new Node(key, value, lv);
				Node* n = m_free_queue.node_queue.front();
				for (int i = 0; i < max_threads; ++i) {
					if ((epoch_array[i * 16] != 0) && (epoch_array[i * 16] < n->ebr_number)) {
						return new Node(key, value, lv);
					}
				}
				m_free_queue.node_queue.pop();
				n->k = key;
				n->v = value;
				for (int i = 0; i <= n->level; ++i) {
					n->next[i].set_ptr(nullptr);
				}
				n->level = lv;
				//std::cout << key << "재사용\n";
				return n;
			}
			void reuse(Node* node) {
				node->ebr_number = epoch_counter;
				m_free_queue.node_queue.emplace(node);
			}
		};

		class Free_queue {
		public:
			std::queue<Node*> node_queue;
			Free_queue() {
				//std::cout << "생성자[" << std::this_thread::get_id() << "] type: " << typeid(*this).name() << '\n';
			}
			~Free_queue() {
				std::cout << "소멸자[" << std::this_thread::get_id() << "] type: " << typeid(*this).name() << " size: "<< node_queue.size() << '\n';
				while (node_queue.size()) {
					Node* n = node_queue.front();
					delete n;
					node_queue.pop();
				}
			}
		};

		class LF_skiplist {
		private:
			EBR ebr;
			Node head, tail;
		public:
			LF_skiplist() {
				head.k = 0;
				head.level = max_level;
				if (std::numeric_limits<Key>::max() > 0) {
					tail.k = std::numeric_limits<Key>::max();
				}
				else {
					tail.k = std::numeric_limits<unsigned long long>::max();
				}
				tail.level = max_level;
				for (auto& p : head.next)
				{
					p.set_ptr(&tail);
				}
				//std::cout << "LF_skiplist 생성자\n";
			}
			~LF_skiplist() {}

			bool find(const Key& key, Node* prevs[], Node* currs[]) {
				ebr.start_epoch();
				Node* prev, *curr, *curr_next;
				bool removed = false;
			retry:
				while (true) {
					prev = &head;
					for (int level = max_level; level >= 0; --level) {
						curr = prev->next[level].get_ptr(removed);
						curr_next = curr->next[level].get_ptr(removed);
						while (removed == true) {
							// 죽은 노드들 연결 끊어주기~
							if (prev->next[level].CAS(curr, curr_next, false, false) == false) {
								goto retry;
							}
							curr = prev->next[level].get_ptr(removed);
							curr_next = curr->next[level].get_ptr(removed);
						}

						if (curr->k < key) {
							prev = curr;
							++level;
							continue;
						}
						prevs[level] = prev;
						currs[level] = curr;
					}
					Key curr_key = curr->k;
					ebr.end_epoch();
					return curr_key == key;
				}
			}

			bool insert(const Key& key, const Value& value) {
				ebr.start_epoch();
				Node* prevs[max_level + 1]{ nullptr, };
				Node* currs[max_level + 1]{ nullptr, };
				Node* n = ebr.get_node(key, value);
				while (true) {
					bool found = find(key, prevs, currs);
					if (found) {
						ebr.end_epoch();
						delete n;
						return false;
					}
					for (int level = 0; level <= n->level; ++level) {
						n->next[level].set_ptr(currs[level]);
					}
					if (false == prevs[0]->next[0].CAS(currs[0], n, false, false))
						continue;
					for (int level = 1; level <= n->level; ++level) {
						while (true) {
							if (prevs[level]->next[level].CAS(currs[level], n, false, false)) break;
							find(key, prevs, currs);
						}
					}
					ebr.end_epoch();
					return true;
				}

			}
			bool remove(const Key& key) {
				ebr.start_epoch();
				Node* prevs[max_level + 1]{ nullptr, };
				Node* currs[max_level + 1]{ nullptr, };
				Node* succ = nullptr;
				bool removed = false;
				bool found = find(key, prevs, currs);
				if (found == false) {
					ebr.end_epoch();
					//std::cout << "존재하지 않는 " << key << std::endl;
					return false;
				}
				for (int level = currs[0]->level; level > 0; --level) {
					succ = currs[0]->next[level].get_ptr(removed);
					while (removed == false) {
						currs[0]->next[level].CAS(succ, succ, false, true);
						succ = currs[0]->next[level].get_ptr(removed);
					}
				}

				succ = currs[0]->next[0].get_ptr(removed);
				if (currs[0]->next[0].CAS(succ, succ, false, true)) {
					Node* temp = currs[0];
					find(key, prevs, currs);
					ebr.reuse(temp);
					ebr.end_epoch();
					return true;
				}
				//std::cout << "마킹이 실패" << key << std::endl;
				ebr.end_epoch();
				return false;
			}

			bool contains(const Key& key) {
				ebr.start_epoch();
				bool removed = false;
				Node* prev{ &head };
				Node* curr{ nullptr };
				Node* succ{ nullptr };
				for (int level = max_level; level >= 0; --level) {
					curr = prev->next[level].get_ptr(removed);
					while (true) {
						succ = curr->next[level].get_ptr(removed);
						while (removed == true) {
							curr = curr->next[level].get_ptr(removed);
							succ = curr->next[level].get_ptr(removed);
						}
						if (curr->k < key) {
							prev = curr;
							curr = succ;
						}
						else break;
					}
				}
				auto result = curr->k;
				ebr.end_epoch();
				return (result == key);
			}

			void print() {
				Node* c = &head;
				bool removed = false;
				while (c != &tail) {
					std::cout << c->k;
					if (removed) {
						std::cout << " (removed)";
					}
					std::cout << " -> ";
					c = c->next[0].get_ptr(removed);
				}
			}
		};

		static constexpr char bucket_size = 11;

		LF_skiplist buckets[bucket_size];

		std::hash<Key> hash_func;

		void set_thread_id() {
			if (m_thread_id == 0) {
				int id = n_threads++;
				m_thread_id = id;
			}
		}
	public:
		LF_hash_skiplist() {
			//std::cout << "LF_hash_skiplist 생성자\n";
		}
		~LF_hash_skiplist() {
			//std::cout << "LF_hash_skiplist 소멸자\n";
		}

		bool Find(const Key& key) {
			set_thread_id();
			unsigned char index = hash_func(key) % bucket_size;
			return buckets[index].contains(key);
		}

		bool Insert(const Key& key, const Value& value) {
			set_thread_id();
			unsigned char index = hash_func(key) % bucket_size;
			return buckets[index].insert(key, value);
			//std::cout << m_thread_id << '\n';
			//Node* n = new Node(key, value, rand() % (max_level + 1));
		}

		bool Remove(const Key& key) {
			set_thread_id();
			unsigned char index = hash_func(key) % bucket_size;
			return buckets[index].remove(key);
		}

		void Print() {
			for (int i = 0; i < bucket_size; ++i) {
				std::cout << "===========" << i << "===========\n";
				buckets[i].print();
				std::cout << std::endl;
			}
		}

		/*bool find(const Key& key, Node* preds[], Node* currs[]) {
			Node* prev = &head;
			Node* next = nullptr;
			for (int level = max_level; level >= 0; --level) {
				next = prev->next[level];

				if (next->key < key) {
					prev = next;
					++level;
					continue;
				}
				preds[level] = prev;
				currs[level] = next;
			}
			return next->key == key;
		}

		bool insert(const Key& key, const Value& value) {
			Node* n = new Node(key, value, rand() % (max_level + 1));
			Node* pred[max_level + 1]{ nullptr, };
			Node* curr[max_level + 1]{ nullptr, };
			while (true) {
				bool found = find(key, pred, curr);
				if (found) {
					delete n;
					return false;
				}
				for (int level = 0; level <= n->level; ++level) {
					n->next[level] = curr[level];
				}
				if (std::atomic_compare_exchange_strong(reinterpret_cast<volatile std::atomic_llong*>(&(pred[0]->next[0])), reinterpret_cast<long long*>(&curr[0]), reinterpret_cast<long long>(n)) == false) {
					continue;
				}
				for (int level = 1; level <= n->level; ++level) {
					while (true) {
						if (std::atomic_compare_exchange_strong(reinterpret_cast<volatile std::atomic_llong*>(&(pred[level]->next[level])), reinterpret_cast<long long*>(&curr[level]), reinterpret_cast<long long>(n))) {
							break;
						}
						find(key, pred, curr);
					}
				}
				return true;
			}*/

		static thread_local Free_queue m_free_queue;
		static thread_local int m_thread_id;
	};
	template<typename Key, typename Value>
	thread_local typename LF_hash_skiplist<Key, Value>::Free_queue LF_hash_skiplist<Key,Value>::m_free_queue;

	template<typename Key, typename Value>
	thread_local int LF_hash_skiplist<Key, Value>::m_thread_id;
}
