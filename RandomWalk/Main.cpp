# include <Siv3D.hpp>
# include <vector>
# include <algorithm>

constexpr double G_Dist = 30.0;

double MaxElemDist = 0.0;
constexpr double DifMEDist = 2.0;


struct Elem {
public:
	static const double R;
	Elem() = default;
	Elem(const Vec3& pos_) :
		pos(pos_) {}
	Vec3 pos;

	void Draw() {
		static Sphere sphere;
		sphere.setPos(pos);
		sphere.setSize(R);
		sphere.draw(HueToColor(atan2(pos.y, pos.z) / 6.28 * 360.0));
	}

	void DrawWhite() {
		static Sphere sphere;
		sphere.setPos(pos);
		sphere.setSize(R);
		sphere.draw();
	}

	void Move() {
		pos += RandomVec3(R * 2.0);

		if (pos.distanceFrom(Vec3::Zero) > G_Dist) {
			auto dir = pos.normalized();
			pos = dir * (G_Dist - 0.5);
		}
	}

	bool IsCollision(Elem* b) {
		return Geometry3D::Intersect(Sphere(pos, R), Sphere(b->pos, b->R));
	}
};

const double Elem::R = 0.2;


std::vector<Elem*> elems;

const double AreaR = G_Dist + 2.0;
Vec3 MAX(AreaR, AreaR, AreaR);
Vec3 MIN(-AreaR, -AreaR, -AreaR);
const int D = 4;
const int LENGTH = 1024;

Elem* areas[D][D][D][LENGTH] = { {{{ nullptr }}} };
Box areaBoxes[D][D][D];

Box GetBoxFromMinMax(const Vec3& min, const Vec3& max) {
	return Box((min + max) * 0.5, max - min);
}

void EmplaceNewElem(Elem* const ptr) {
	elems.emplace_back(ptr);
	MaxElemDist = Max(ptr->pos.distanceFrom(Vec3::Zero), MaxElemDist);

	for (size_t i = 0; i < D; i++)
	{
		for (size_t j = 0; j < D; j++)
		{
			for (size_t k = 0; k < D; k++)
			{
				auto area = areaBoxes[i][j][k];
				if (Geometry3D::Intersect(area, Box(ptr->pos, ptr->R * 2.0))) {
					for (size_t l = 0; l < LENGTH; l++)
					{
						if (areas[i][j][k][l] == nullptr) {
							areas[i][j][k][l] = ptr;
							return;
						}
					}
				}
			}
		}
	}
}

Elem* IsCollision(Elem* ptr) {

	for (size_t i = 0; i < D; i++)
	{
		for (size_t j = 0; j < D; j++)
		{
			for (size_t k = 0; k < D; k++)
			{
				auto area = areaBoxes[i][j][k];
				if (Geometry3D::Intersect(area, Box(ptr->pos, ptr->R * 2.0))) {
					for (size_t l = 0; l < LENGTH; l++)
					{
						Elem* tar = areas[i][j][k][l];
						if (tar == nullptr) {
							break;
						}
						if (tar->IsCollision(ptr)) {
							return tar;
						}
					}
				}
			}
		}
	}
	return nullptr;
}

// 軌跡描画用クラス
// pointsを描画、subPointsは
class Trail {
public:

	class Points {
	public:
		std::vector<Vec3> vecs;
		size_t index;
		static const int MAX;


		Points() :
			index(0)
		{
			vecs.resize(MAX);
		}

		void Reset() {
			index = 0;
		}

		void Emplace(const Vec3& point) {
			vecs[index] = point;
			index++;
		}

		void Copy(const Points& rhs) {
			for (size_t i = 0; i < rhs.index; i++) {
				vecs[i] = rhs.vecs[i];
			}
			index = rhs.index;
		}
	};

	static const double R;

	Points points;
	Points buffer;

	void Draw() {
		static Sphere sphere;
		for (size_t i = 0; i < points.index; i++) {
			sphere.setPos(points.vecs[i]);
			sphere.setSize(R);
			sphere.draw(HueToColor(i * 360.0 / 10000.0));
		}
	}

	void EmplaceBuffer(const Vec3& point) {
		buffer.Emplace(point);
	}

	void Reset() {
		points.Reset();
	}

	void ResetBuffer() {
		buffer.Reset();
	}

	void BufferToMain() {
		points.Copy(buffer);
	}
};


const double Trail::R = 0.2;
const int Trail::Points::MAX = 80000;

void Main()
{
	for (size_t i = 0; i < D; i++)
	{
		for (size_t j = 0; j < D; j++)
		{
			for (size_t k = 0; k < D; k++)
			{
				Vec3 min(
					Lerp(MIN.x, MAX.x, (double)i / D),
					Lerp(MIN.y, MAX.y, (double)j / D),
					Lerp(MIN.z, MAX.z, (double)k / D));
				Vec3 max(
					Lerp(MIN.x, MAX.x, (double)(i + 1) / D),
					Lerp(MIN.y, MAX.y, (double)(j + 1) / D),
					Lerp(MIN.z, MAX.z, (double)(k + 1) / D));
				areaBoxes[i][j][k] = GetBoxFromMinMax(min, max);
			}
		}
	}

	elems.reserve(11000);


	// 最初の一点を生成
	EmplaceNewElem(new Elem(RandomVec3(0.0)));

	bool endFrag = false;

	//Trail trail;

	const Font font(14);

	Graphics3D::SetCamera(Camera(
		Vec3(0.0, 0.0, -10.0), Vec3::Zero, Vec3(0.0, 1.0, 0.0), 60.0, 0.01));


	while (System::Update())
	{
		font(L"NUM : " + std::to_wstring(elems.size())).draw(Vec2(10.0, 10.0));
		font(Profiler::FPS(), L"fps").draw(Vec2(10.0, 30.0));

		//trail.ResetBuffer();

		if (elems.size() < 29999 && !endFrag) {
			Elem elem(Vec3::Right * G_Dist);
			//Elem elem(RandomVec3(G_Dist));
			elem.Draw();

			for (size_t i = 0; i < 40000; i++)
			{
				elem.Move();
				//trail.EmplaceBuffer(elem.pos);

				auto e = IsCollision(&elem);
				if (e) {
					auto dir = (elem.pos - e->pos).normalize();
					elem.pos = e->pos + dir * (e->R + elem.R);
					EmplaceNewElem(new Elem(elem));
					if (elem.pos.distanceFrom(Vec3::Zero) > G_Dist - 0.5) {
						endFrag = true;
					}
					//trail.BufferToMain();
					break;
				}
			}

			// elem.Draw();
		}

		for (auto&& i : elems) {
			i->Draw();
			//i->DrawWhite();
		}

		//trail.Draw();

		if (Input::KeyC.pressed) {

			for (size_t i = 0; i < D; i++)
			{
				for (size_t j = 0; j < D; j++)
				{
					for (size_t k = 0; k < D; k++)
					{
						auto b = areaBoxes[i][j][k];
						b.size *= 0.99;
						b.draw();
					}
				}
			}
		}
		if (Input::KeyV.pressed) {
			Sphere(Vec3::Zero, G_Dist).draw();
		}

		Graphics3D::FreeCamera();
	}
}
