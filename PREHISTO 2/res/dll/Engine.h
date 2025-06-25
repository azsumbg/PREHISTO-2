#pragma once

#ifdef ENGINE_EXPORTS
#define ENGINE_API __declspec(dllexport)
#else
#define ENGINE_API __declspec(dllimport)
#endif

#include <random>
#include <type_traits>

enum class types {
	evil1 = 0, evil2 = 1, evil3 = 2, evil4 = 3, evil5 = 4, evil6 = 5,
	hero = 6, axe = 7, fire = 8, no_type = 9
};
enum class assets { platform1 = 0, platform2 = 1, platform3 = 2, potion = 3, gold = 4, tree = 5, field = 6, no_type = 7 };
enum class dirs { stop = 0, up = 1, down = 2, left = 3, right = 4, up_left = 5, up_right = 6,
	down_left = 7, down_right = 8 };
enum class states { stop = 0, move = 1, attack = 2, gather = 3, fall = 4, melee = 5, attack_finished = 6 };

constexpr float scr_width{ 800.0f };
constexpr float scr_height{ 750.0f };
constexpr float sky{ 50.0f };
constexpr float ground{ 650.0f };

struct FPOINT
{
	float x{ 0 };
	float y{ 0 };
};

namespace dll
{
	template<typename T> class BAG
	{
		private:
			T* m_ptr{ nullptr };
			size_t max_size{ 0 };
			size_t next_pos{ 0 };
			bool has_elements{ false };

		public:
			BAG() :max_size{ 1 }, m_ptr{ reinterpret_cast<T*>(calloc(1,sizeof(T))) }
			{
				has_elements = false;
			}
			BAG( size_t lenght ) : max_size{ lenght }, m_ptr{ reinterpret_cast<T*>(calloc(lenght,sizeof(T))) }
			{
				has_elements = false;
			}
			~BAG()
			{
				if (m_ptr)free(m_ptr);
			}

			size_t capacity() const
			{
				return max_size;
			}
			size_t size() const
			{
				return next_pos;
			}
			bool is_valid() const
			{
				if (m_ptr)return true;
				return false;
			}
			bool empty() const
			{
				return !(next_pos > 0);
			}

			void push_back(T element)
			{
				if (m_ptr)
				{
					has_elements = true;

					if (next_pos < max_size)
					{
						m_ptr[next_pos] = element;
						++next_pos;
						return;
					}
					else
					{
						++max_size;
						m_ptr = reinterpret_cast<T*>(realloc(m_ptr, max_size * sizeof(T)));
						if (m_ptr)
						{
							m_ptr[next_pos] = element;
							++next_pos;
						}
						return;
					}
				}
			}
			void push_front(T element)
			{
				if (m_ptr)*m_ptr = element;
				has_elements = true;
			}

			T& front() const
			{
				T dummy{};
				if (m_ptr)return (*m_ptr);
				return dummy;
			}
			T& back() const
			{
				T dummy{};
				if (m_ptr)
				{
					if (next_pos > 0)return m_ptr[next_pos - 1];
					return (*m_ptr);
				}
				return dummy;
			}

			T& operator[](size_t index)
			{
				T dummy{};

				if (index <= next_pos)
				{
					if (index == 0)return (*m_ptr);
					else if (index < next_pos)return m_ptr[index];
				}

				return dummy;
			}
			void operator()(size_t index, T element)
			{
				if (index <= next_pos)
				{
					if (index == 0)(*m_ptr) = element;
					else if (index < next_pos)m_ptr[index] = element;
					
					return;
				}
				push_back(element);
			}
	};
	
	class ENGINE_API RANDIt
	{
		private:
			std::random_device rd{};
			std::seed_seq* sq = { nullptr };
			std::mt19937* twister{ nullptr };

		public:

			RANDIt();
			~RANDIt();

			int operator()(int min, int max);
	};

	class ENGINE_API PROTON
	{
	protected:
		float _width{ 0 };
		float _height{ 0 };

	public:
		FPOINT start{};
		FPOINT end{};
		FPOINT center{};

		float x_radius{ 0 };
		float y_radius{ 0 };

		PROTON();
		PROTON(FPOINT start_point, float width, float height);
		PROTON(float sx, float sy, float width, float height);
		virtual ~PROTON() {};

		void SetEdges();
		void NewDims(float new_width, float new_height);
		void NewWidth(float new_width);
		void NewHeight(float new_height);

		float GetWidth() const;
		float GetHeight() const;
	};

	class ENGINE_API FIELD :public PROTON
	{
	private:

		float speed = 1.0f;

		FIELD(assets _type, float _sx, float _sy);

	public:
		dirs dir{ dirs::left };
		assets type;

		FIELD();

		virtual ~FIELD() {};

		bool Move(float gear, dirs to_where);
		void Release();

		friend ENGINE_API FIELD* FieldFactory(assets mytype, float sx, float sy);
	};

	class ENGINE_API CREATURE :public PROTON
	{
		private:
			bool hor_line{ false };
			bool vert_line{ false };

			float move_sx{ 0 };
			float move_sy{ 0 };
			float move_ex{ 0 };
			float move_ey{ 0 };

			float slope{ 0 };
			float intercept{ 0 };

			int frame{ 0 };
			int max_frames{ 0 };
			int frame_delay{ 0 };

			float speed = 0.5f;

			float sight_limit{ 0 };
			int attack_delay{ 0 };
			int max_attack_delay{ 0 };

			bool jump_start = false;
			bool jump_up = false;

			void SetPathInfo(float to_x, float to_y);

			CREATURE(types _type, float _sx, float _sy, float _targ_x, float _targ_y);

		public:
			types type;
			dirs dir = dirs::stop;
			states state = states::stop;
			bool jump{ false };
			int lifes = 0;

			virtual ~CREATURE() {};

			int GetFrame();
	
			bool Contact(FIELD& what, dirs& where);

			void Move(float gear);
			bool Move(float gear, float targ_x, float targ_y);

			states Dispatcher(FPOINT hero_point, BAG<FIELD>& objects);

			void Release();

			friend ENGINE_API CREATURE* CreatureFactory(types mytype, float sx, float sy, float targ_x, float targ_y);
	};

	// FUNCTIONS DECLARATION *******************************

	float Distance(FPOINT MyPoint, FPOINT TargetPoint);
	bool Sort(BAG<FPOINT>& objects, FPOINT target);

	template<typename T> bool Sort(BAG<T>& objects, bool reversed_order)
	{
		if constexpr (!std::is_same<T, int>::value && !std::is_same<T, double>::value && !std::is_same<T, float>::value
			&& !std::is_same<T, long>::value && !std::is_same<T, short>::value && !std::is_same<T, char>::value
			&& !std::is_same<T, wchar_t>::value)return false;

		if (objects.size() < 2)return false;

		bool is_ok{ false };

		if (!reversed_order)
		{
			while (!is_ok)
			{
				is_ok = true;

				for (size_t i = 0; i < objects.size() - 1; ++i)
				{
					if (objects[i] > objects[i + 1])
					{
						T temp = objects[i];
						objects[i] = objects[i + 1];
						objects[i + 1] = temp;
						is_ok = false;
						break;
					}
				}
			}
		}
		else
		{
			while (!is_ok)
			{
				is_ok = true;

				for (size_t i = 0; i < objects.size() - 1; ++i)
				{
					if (objects[i] < objects[i + 1])
					{
						T temp = objects[i];
						objects[i] = objects[i + 1];
						objects[i + 1] = temp;
						is_ok = false;
						break;
					}
				}
			}
		}

		return true;
	}

	typedef FIELD* Asset;
	typedef CREATURE* Creature;

	ENGINE_API FIELD* FieldFactory(assets mytype, float sx, float sy);
	ENGINE_API CREATURE* CreatureFactory(types mytype, float sx, float sy, float targ_x = 0, float targ_y = 0);
}