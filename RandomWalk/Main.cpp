# include <Siv3D.hpp>
# include <vector>
# include <algorithm>


struct Elem {
public:
	static const double R;
	Elem() = default;
	Elem(const Vec3& pos_) :
		pos(pos_) {}
	Vec3 pos;

	void Draw() {
		//Sphere(pos, R).draw();
		static Sphere sphere;
		sphere.setPos(pos);
		sphere.setSize(R);
		sphere.draw(HueToColor(atan2(pos.y, pos.z) / 6.28 * 360.0));
	}

	void Move() {
		pos += RandomVec3(R);
		if (pos.distanceFrom(Vec3::Zero) > 10.0) {
			auto dir = pos.normalize();
			pos = dir * 10.0;
		}
	}

	bool IsCollision(const Elem& b) {
		return Geometry3D::Intersect(Sphere(pos, R), Sphere(b.pos, b.R));
	}
};

const double Elem::R = 0.2;

void Main()
{
	const Font font(14);

	std::vector<Elem> elems;
	elems.reserve(11000);

	for (size_t i = 0; i < 1; i++)
	{
		elems.emplace_back(Elem(RandomVec3(0.0)));
	}
	Graphics3D::SetCamera(Camera(
		Vec3(0.0, 0.0, -10.0), Vec3::Zero, Vec3(0.0, 1.0, 0.0), 60.0, 0.01));

	double rad = 0.0;

	while (System::Update())
	{
		font(L"NUM : " + std::to_wstring(elems.size())).draw(Vec2(10.0, 10.0));
		font(Profiler::FPS(), L"fps").draw(Vec2(10.0, 30.0));

		if (elems.size() < 700) {
			Elem elem(RandomVec3(10.0));
			// Elem elem(Vec3(10.0, 0.0, 0.0));
			elem.Draw();

			bool breakFrag = false;
			for (size_t i = 0; i < 10000; i++)
			{
				elem.Move();

				for (auto&& e : elems) {
					if (elem.IsCollision(e)) {
						auto dir = (elem.pos - e.pos).normalize();
						elem.pos = e.pos + dir * (e.R + elem.R);
						elems.emplace_back(elem);
						breakFrag = true;
						break;
					}
				}
				if (breakFrag) {
					break;
				}

			}

			elem.Draw();
		}
		for (auto&& i : elems) {
			i.Draw();
		}
		Graphics3D::FreeCamera();
	}
}
