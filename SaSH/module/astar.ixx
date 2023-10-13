﻿module;

#include <QPoint>

export module astar;

import <memory_resource>;
import <functional>;
import <algorithm>;
import <ranges>;
import <mutex>;
import <vector>;
import <memory>;

namespace AStar
{
	constexpr __int64 kStepValue = 24;
	constexpr __int64 kObliqueValue = 32;

	export using CanPassCallback = std::function<bool(const QPoint&)>;

	export class Device
	{
	private:
		/**
		 * 路徑節點狀態
		 */
		typedef enum
		{
			NOTEXIST,               // 不存在
			IN_OPENLIST,            // 在開啟列表
			IN_CLOSEDLIST           // 在關閉列表
		}NodeState;

		/**
		 * 路徑節點
		 */
		struct Node
		{
			__int64      g;          // 與起點距離
			__int64      h;          // 與終點距離
			QPoint      pos;        // 節點位置
			NodeState   state;      // 節點狀態
			Node* parent;     // 父節點

			/**
			 * 計算f值
			 */
			inline __int64 __fastcall f() const { return g + h; }

			inline Node(const QPoint& pos)
				: g(0), h(0), pos(pos), parent(nullptr), state(NodeState::NOTEXIST)
			{
			}
		};

	public:
		Device()
			: step_val_(kStepValue)
			, oblique_val_(kObliqueValue)

		{

		}

		virtual ~Device()
		{
			clear();
		}

	public:
		/**
		 * 獲取直行估值
		 */
		constexpr __int64 __fastcall get_step_value() const
		{
			return step_val_;
		}

		/**
		 * 獲取拐角估值
		 */
		constexpr __int64 __fastcall get_oblique_value() const
		{
			return oblique_val_;
		}

		/**
		 * 設置直行估值
		 */
		constexpr void __fastcall set_step_value(__int64 value)
		{
			step_val_ = value;
		}

		/**
		 * 獲取拐角估值
		 */
		constexpr void __fastcall set_oblique_value(__int64 value)
		{
			oblique_val_ = value;
		}

		void __fastcall set_canpass(const CanPassCallback& callback)
		{
			can_pass_ = callback;
		}

		void __fastcall set_corner(bool corner)
		{
			corner_ = corner;
		}


		/**
		 * 初始化參數
		 */
		void __fastcall init(__int64 width, __int64 height);

		/**
		 * 執行尋路操作
		 */
		bool __fastcall find(const QPoint& start, const QPoint& end, std::vector<QPoint>* pPaths);

	private:
		/**
		 * 清理參數
		 */
		void clear()
		{
			// 释放 mapping_ 中的节点内存
			for (Node*& node : record_)
			{
				if (node)
				{
					std::allocator_traits<std::pmr::polymorphic_allocator<Node>>::destroy(*allocator_, node);
					allocator_->deallocate(node, 1);
				}
			}
			open_list_.clear();
			record_.clear();
			std::fill(mapping_.begin(), mapping_.end(), nullptr);
		}

		/**
		 * 參數是否有效
		 */
		bool __fastcall is_vlid_params(const QPoint& start, const QPoint& end) const
		{
			return ((can_pass_ != nullptr)
				&& ((start.x() >= 0) && (start.x() < width_))
				&& ((start.y() >= 0) && (start.y() < height_))
				);
		}

	private:
#if defined(Chebyshev_distance)
		__forceinline __int64 __fastcall Chebyshev_Distance(const QPoint& current, const QPoint& end)
		{
			return std::max(std::abs(current.x() - end.x()), std::abs(current.y() - end.y()));
		}
#elif defined(Euclidean_distance)
		__forceinline __int64 __fastcall Euclidean_Distance(const QPoint& current, const QPoint& end)
		{
			return std::sqrt(std::pow(current.x() - end.x(), 2) + std::pow(current.y() - end.y(), 2));
		}
#elif defined(Octile_distance)
		__forceinline __int64 __fastcall Octile_Distance(const QPoint& current, const QPoint& end)
		{
			quint64  dx = std::abs(current.x() - end.x());
			quint64  dy = std::abs(current.y() - end.y());

			if (dx > dy)
				return kStepValue * dx + (kObliqueValue - kStepValue) * dy;
			return kStepValue * dy + (kObliqueValue - kStepValue) * dx;
		}
#else 
		__forceinline __int64 __fastcall Manhattan_Distance(const QPoint& current, const QPoint& end)
		{
			return (current - end).manhattanLength();
		}
#endif

		/**
		 * 二叉堆上濾
		 */
		void __fastcall percolate_up(__int64& hole)
		{
			__int64 parent = 0;
			while (hole > 0)
			{
				parent = (hole - 1) / 2;
				if (open_list_[hole]->f() < open_list_[parent]->f())
				{
#if _MSVC_LANG > 201703L
					std::ranges::swap(open_list_[hole], open_list_[parent]);
#else
					std::swap(open_list_[hole], open_list_[parent]);
#endif
					hole = parent;
				}
				else
				{
					return;
				}
			}
		}

		/**
		 * 獲取節點索引
		 */
		bool __fastcall get_node_index(Node*& node, __int64* index)
		{
			*index = 0;
			__int64 size = open_list_.size();
			while (*index < size)
			{
				if (open_list_[*index]->pos == node->pos)
				{
					return true;
				}
				++(*index);
			}
			return false;
		}

		/**
		 * 計算G值
		 */
		__forceinline __int64 __fastcall calcul_g_value(Node*& parent, const QPoint& current)
		{
#if defined(Chebyshev_distance)
			__int64 g_value = Chebyshev_Distance(current, parent->pos) == 2 ? kObliqueValue : kStepValue;
			return g_value += parent->g;
#elif defined(Euclidean_distance)
			__int64 g_value = Euclidean_Distance(current, parent->pos) == 2 ? kObliqueValue : kStepValue;
			return g_value += parent->g;
#elif defined(Octile_distance)
			__int64 g_value = Octile_Distance(current, parent->pos) == 2 ? kObliqueValue : kStepValue;
			return g_value += parent->g;
#else
			__int64 g_value = Manhattan_Distance(current, parent->pos) == 2 ? kObliqueValue : kStepValue;
			g_value += parent->g;
			return g_value;
#endif
		}

		/**
		 * 計算F值
		 */
		__forceinline __int64 __fastcall calcul_h_value(const QPoint& current, const QPoint& end)
		{
#if defined(Chebyshev_distance)
			__int64 h_value = Chebyshev_Distance(current, end);
#elif defined(Euclidean_distance)
			__int64 h_value = Euclidean_Distance(current, end);
#elif defined(Octile_distance)
			__int64 h_value = Octile_Distance(current, end);
#else
			__int64 h_value = Manhattan_Distance(end, current);
#endif
			return h_value * kStepValue;
		}

		/**
		 * 節點是否存在於開啟列表
		 */
		__forceinline bool __fastcall in_open_list(const QPoint& pos, Node*& out_node)
		{
			out_node = mapping_[pos.y() * width_ + pos.x()];
			return out_node ? out_node->state == NodeState::IN_OPENLIST : false;
		}

		/**
		 * 節點是否存在於關閉列表
		 */
		__forceinline bool __fastcall in_closed_list(const QPoint& pos)
		{
			Node* node_ptr = mapping_[pos.y() * width_ + pos.x()];
			return node_ptr ? node_ptr->state == NodeState::IN_CLOSEDLIST : false;
		}

		/**
		 * 是否可通過
		 */
		bool __fastcall can_pass(const QPoint& pos)
		{
			return (pos.x() >= 0 && pos.x() < width_ && pos.y() >= 0 && pos.y() < height_) ? can_pass_(pos) : false;
		}

		/**
		 * 當前點是否可到達目標點
		 */
		bool __fastcall can_pass(const QPoint& current, const QPoint& destination, const bool& allow_corner)
		{
			if (destination.x() >= 0 && destination.x() < width_ && destination.y() >= 0 && destination.y() < height_)
			{
				if (in_closed_list(destination))
					return false;
#if defined(Chebyshev_distance)
				if (Chebyshev_Distance(destination, current) == 1)
#elif defined(Euclidean_distance)
				if (Euclidean_Distance(destination, current) == 1)
#elif defined(Octile_distance)
				if (Octile_Distance(destination, current) == 1)
#else
				if (Manhattan_Distance(destination, current) == 1)
#endif
				{
					return can_pass_(destination);
				}
				else if (allow_corner)
				{
					return (can_pass_(destination)) &&
						(can_pass(QPoint(current.x() + destination.x() - current.x(), current.y()))) &&
						(can_pass(QPoint(current.x(), current.y() + destination.y() - current.y())));
				}
			}
			return false;
		}

		/**
		 * 查找附近可通過的節點
		 */
		void __fastcall find_can_pass_nodes(const QPoint& current, const bool& corner, std::vector<QPoint>* out_lists)
		{
			QPoint destination;
			__int64 row_index = static_cast<__int64>(current.y()) - 1;
			const __int64 max_row = static_cast<__int64>(current.y()) + 1;
			const __int64 max_col = static_cast<__int64>(current.x()) + 1;

			if (row_index < 0)
			{
				row_index = 0;
			}

			while (row_index <= max_row)
			{
				__int64 col_index = static_cast<__int64>(current.x()) - 1;

				if (col_index < 0)
				{
					col_index = 0;
				}

				while (col_index <= max_col)
				{
					destination.setX(col_index);
					destination.setY(row_index);
					if (can_pass(current, destination, corner))
					{
						out_lists->emplace_back(destination);
					}
					++col_index;
				}
				++row_index;
			}
		}

		/**
		 * 處理找到節點的情況
		 */
		void __fastcall handle_found_node(Node*& current, Node*& destination)
		{
			__int64 g_value = calcul_g_value(current, destination->pos);
			if (g_value < destination->g)
			{
				destination->g = g_value;
				destination->parent = current;

				__int64 index = 0;
				if (get_node_index(destination, &index))
				{
					percolate_up(index);
				}
				else
				{
					assert(false);
				}
			}
		}

		/**
		 * 處理未找到節點的情況
		 */
		void __fastcall handle_not_found_node(Node*& current, Node*& destination, const QPoint& end)
		{
			destination->parent = current;
			destination->h = calcul_h_value(destination->pos, end);
			destination->g = calcul_g_value(current, destination->pos);

			destination->state = NodeState::IN_OPENLIST;
			mapping_[destination->pos.y() * width_ + destination->pos.x()] = destination;

			open_list_.emplace_back(destination);
			while (open_list_.empty())
			{
				open_list_.emplace_back(destination);
			}

#if _MSVC_LANG > 201703L
			std::ranges::push_heap(open_list_, [](const Node* a, const Node* b)->bool
#else
			std::push_heap(open_list_.begin(), open_list_.end(), [](const Node* a, const Node* b)->bool

#endif
				{ return a->f() > b->f(); });
		}

	private:
		bool					corner_ = true;
		__int64                  step_val_;
		__int64                  oblique_val_;
		__int64                  height_ = 0;
		__int64                  width_ = 0;
		CanPassCallback         can_pass_ = nullptr;
		QPoint					start_;
		QPoint					end_;
		std::mutex			    mutex_;
		std::vector<Node*>      mapping_;
		std::vector<Node*>      open_list_;
		std::vector<Node*>      record_;

#if _MSVC_LANG >= 201703L
		std::unique_ptr<std::pmr::monotonic_buffer_resource> resource_;
		std::unique_ptr<std::pmr::polymorphic_allocator<Node>> allocator_;
#endif
	};

	/**
	* 初始化參數
	*/
	void __fastcall Device::init(__int64 width, __int64 height)
	{
		std::lock_guard<std::mutex> lock(mutex_);
		if (height_ == height && width_ == width)
			return;

		height_ = height;
		width_ = width;
		resource_.reset(new std::pmr::monotonic_buffer_resource(width_ * height_ * sizeof(Node) * 2));
		allocator_.reset(new std::pmr::polymorphic_allocator<Node>(resource_.get()));
		open_list_.clear();
		mapping_.clear();
		mapping_.resize(width_ * height_);
	}

	/**
	* 執行尋路操作
	*/
	bool __fastcall Device::find(const QPoint& start, const QPoint& end, std::vector<QPoint>* pPaths)
	{
		std::lock_guard<std::mutex> lock(mutex_);
		if (pPaths)
			pPaths->clear();

		if (!is_vlid_params(start, end))
		{
			clear();
			return false;
		}

		start_ = start;
		end_ = end;

		std::vector<QPoint> nearby_nodes;
		nearby_nodes.reserve(corner_ ? 8 : 4);
		constexpr size_t alloc_size(1u);
		// 將起點放入開啟列表
		Node* start_node = allocator_->allocate(alloc_size);  // 分配內存
		record_.emplace_back(start_node);
		std::allocator_traits<std::pmr::polymorphic_allocator<Node>>::construct(*allocator_, start_node, start_);// 構造對象
		open_list_.emplace_back(start_node);
		start_node->state = NodeState::IN_OPENLIST;
		mapping_[start_node->pos.y() * width_ + start_node->pos.x()] = start_node;

		// 尋路操作
		while (!open_list_.empty())
		{
			// 找出f值最小節點
			Node* current = open_list_.front();
#if _MSVC_LANG > 201703L
			std::ranges::pop_heap(open_list_, [](const Node* a, const Node* b)->bool
#else
			std::pop_heap(open_list_.begin(), open_list_.end(), [](const Node* a, const Node* b)->bool
#endif
				{
					return a->f() > b->f();
				});

			open_list_.pop_back();

			mapping_[current->pos.y() * width_ + current->pos.x()]->state = NodeState::IN_CLOSEDLIST;

			// 是否找到終點
			if (current->pos == end_)
			{
				if (pPaths == nullptr)
				{
					clear();
					return true;
				}

				while (current->parent)
				{
					pPaths->emplace_back(current->pos);
					current = current->parent;
				}

				std::reverse(pPaths->begin(), pPaths->end());
				break;
			}

			// 查找周圍可通過節點
			nearby_nodes.clear();
			find_can_pass_nodes(current->pos, corner_, &nearby_nodes);

			// 計算周圍節點的估值
			size_t index = 0;
			const size_t size = nearby_nodes.size();
			while (index < size)
			{
				Node* next_node = nullptr;
				if (in_open_list(nearby_nodes[index], next_node))
				{
					handle_found_node(current, next_node);
				}
				else
				{
					next_node = allocator_->allocate(alloc_size);  // 分配內存
					record_.push_back(next_node);
					std::allocator_traits<std::pmr::polymorphic_allocator<Node>>::construct(*allocator_, next_node, nearby_nodes[index]);// 構造對象
					handle_not_found_node(current, next_node, end_);
				}
				++index;
			}
		}

		clear();

		if (pPaths != nullptr)
			return !pPaths->empty();
		return false;
	}
}