namespace Firefly
{
	__declspec(dllimport) void Test();
}

int main(int argc, char** argv)
{
	Firefly::Test();
	return 0;
}