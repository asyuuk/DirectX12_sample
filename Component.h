#pragma once
#include<list>
#include<memory>
class Component
{

public: 
	Component();
	virtual ~Component();

	virtual void Update() = 0;
	virtual void Draw() = 0;
	virtual void Start() = 0;

};

class Object
{
public:
	Object() {};
	~Object()
	{
		for (auto& component : components)
		{
			delete component.get();
		}
	}
	;
	std::list<std::shared_ptr<Component>> components;
	template<class T>
	T* AddComponent()
	{
		T* component = new T();
		components.push_back(std::shared_ptr<T>(component));
		component->Start();
		return component;
	}
	template<class T>
	T* GetComponent()
	{
		for (auto& component : components)
		{
			if (dynamic_cast<T*>(component.get()))
			{
				return dynamic_cast<T*>(component.get());
			}
		}
		return nullptr;
	}

	void Update()
	{
		for (auto& component : components)
		{
			component->Update();
		}
	}
	void Draw()
	{
		for (auto& component : components)
		{
			component->Draw();
		}
	}

	void Start()
	{
		for (auto& component : components)
		{
			component->Start();
		}
	}
	
};

