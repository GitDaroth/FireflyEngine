#include <FireflyEngine.h>

class SandboxLayer : public Firefly::Layer
{
public:
	SandboxLayer(const std::string& name, int orderNumber = 0) :
		Layer(name, orderNumber) {}

	~SandboxLayer() {}

	virtual void OnAttach() override
	{
		Firefly::Logger::Debug("Sandbox", "{0}({1}) - Attach", GetName(), GetOrderNumber());
	}

	virtual void OnDetach() override
	{
		Firefly::Logger::Debug("Sandbox", "{0}({1}) - Detach", GetName(), GetOrderNumber());
	}

	virtual void OnUpdate() override
	{
		//Firefly::Logger::Debug("Sandbox", "{0}({1}) - Update", GetName(), GetOrderNumber());
	}

	virtual void OnEvent(std::shared_ptr<Event> event) override
	{
		Firefly::Logger::Debug("Sandbox", "{0}({1}) - {2}", GetName(), GetOrderNumber(), event->ToString());
	}
};

class SandboxApp : public Firefly::Application
{
public:
	SandboxApp()
	{
		//auto layer1 = std::make_shared<SandboxLayer>("Layer1", 0);
		//auto layer2 = std::make_shared<SandboxLayer>("Layer2", 1);
		//auto layer3 = std::make_shared<SandboxLayer>("Layer3", 2);
		//AddLayer(layer1);
		//AddLayer(layer2);
		//AddLayer(layer3);
		//layer2->SetOrderNumber(10);
	}

	~SandboxApp()
	{
	}
};

Firefly::Application* Firefly::InstantiateApplication()
{
	return new SandboxApp();
}