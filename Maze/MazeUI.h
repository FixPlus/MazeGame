#pragma once
#include "../external/imgui/imgui.h"
#include <string>
#include <sstream>
#include <iostream>
#include <list>



/*




	Wrapper above ImGui library
	
	simpifies construction and rendering of UI




*/


namespace MazeUI {



class UIElement { 
 /*

	Base interface for any UI element could be drawn on screen separately


 */

public:
	bool expired = false;
	bool visible = true;
	virtual bool update(int width, int height) = 0;

	virtual ~UIElement() {};
};







class WindowItem {
/*
	

	Base interface for any widget drawn on ImGui Window



*/
public:
	virtual bool update(int width, int height) = 0;

	virtual ~WindowItem() {};
};

class Text: public WindowItem {

/*

	Text labels


*/

protected:
	std::string text;
public:
	Text(std::string const& t = "Sample text"): text(t) {}
	
	bool update(int width, int height) override{
		ImGui::TextUnformatted(text.c_str());
		return false;
	}

};

template<typename Stat>
class StatText: public Text {
/*


	Text lables that diplay some information in next form: "STAT_NAME: STAT"


*/
	Stat const& stat_;
	std::string stat_name_;
public:
	StatText(Stat const& stat, std::string const& stat_name = "Stat"): stat_(stat), stat_name_(stat_name){
		std::ostringstream oss;
		oss << stat_name_ << ": " << stat_;
		text = oss.str();
	}
	bool update(int width, int height) override{
		std::ostringstream oss;
		oss << stat_name_ << ": " << stat_;
		text = oss.str();
		Text::update(width, height);
		return false;
	}
};


class InputBox: public WindowItem{
	std::string label;
	static int DEFAULT_STAT;
	int& refered_stat;
	int lower_bound, upper_bound;
public:
	InputBox(std::string lbl = "input", int& ref = DEFAULT_STAT, int lb = 5, int ub = 100): label(lbl), refered_stat(ref), lower_bound(lb), upper_bound(ub){};

	bool update(int width, int height) override{
		ImGui::InputInt(label.c_str(), &refered_stat);

		if(refered_stat < lower_bound)
			refered_stat = lower_bound;

		if(refered_stat > upper_bound)
			refered_stat = upper_bound;

		return false;
	}
};


class Button: public WindowItem {
/*


	Texted Button that runs onClick() function when tapped


*/
	std::string label;
	float xSize, ySize;
	std::function<void(void)> onClick;
public:
	Button(std::string lbl = "Button", std::function<void(void)> onCl = [](){},float xSz = 0, float ySz = 0): label(lbl), xSize(xSz), ySize(ySz), onClick(onCl) {};

	bool update(int width, int height) override{
		if(ImGui::Button(label.c_str(), ImVec2(xSize * width, ySize * height))){
			onClick();
			return true;
		}
		return false;
	}

};

class Window : public UIElement {
/*

	Standard ImGui window that can contain widgets (items)

*/

	std::string header; // header (aka title) will be shown on the top of the window unless ImGuiWindowFlag_NoTitle flag will be provided
	float xPos, yPos, xWide, yWide; // Position on screen ond size of the window
	enum ImGuiWindowFlags_ flags;

protected:

	std::vector<WindowItem*> items;

	virtual bool updElems(int width, int height){
		bool ret = false;
		for(auto& item: items)
			ret =  ret || item->update(width, height);

		return ret;
	};

public:

	Window(std::string const& head = "Window", float xP = 0.0f, float yP = 0.0f, float xW = 0.1f, float yW = 0.1f, int fl = 0): 
	header(head), xPos(xP), yPos(yP), xWide(xW), yWide(yW), flags(static_cast<enum ImGuiWindowFlags_>(fl))
	{}

	bool update(int width, int height) override{
		
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
		ImGui::SetNextWindowPos(ImVec2(static_cast<float>(width) * xPos, static_cast<float>(height) * yPos));
		ImGui::SetNextWindowSize(ImVec2(static_cast<float>(width) * xWide, static_cast<float>(height) * yWide), ImGuiCond_Always);
		ImGui::Begin(header.c_str(), nullptr, flags);
		bool ret = updElems(width, height);
		ImGui::End();
		ImGui::PopStyleVar();
		return ret;
	}

	void addNewItem(WindowItem* item){

		items.push_back(item);
		//TODO: add exception on NULL argument

	}

	void clear(){
		for(auto& item: items)
			delete item;

		items.resize(0);
	}

	virtual ~Window() {
		clear();
	};
};

class TimedWindow: public Window{
std::chrono::time_point<std::chrono::high_resolution_clock> tStart = std::chrono::high_resolution_clock::now();
float duration;
public:
	TimedWindow(float dur, std::string const& head = "Window", float xP = 0.0f, float yP = 0.0f, float xW = 0.1f, float yW = 0.1f, int fl = 0)
	: Window(head, xP, yP, xW, yW, fl), duration{dur} { tStart = std::chrono::high_resolution_clock::now();};
	bool update(int width, int height) override{
		bool ret = Window::update(width, height);
		auto tNow = std::chrono::high_resolution_clock::now();
		auto tDiff = std::chrono::duration<double, std::milli>(tNow - tStart).count();
		float div = static_cast<float>(tDiff / 1000.0f);
		if(div > duration){
			expired = true;
			return true;
		}
		return ret;
	}
};
class Manager final{
/*
	

	Class that Owns all UIElements 


*/
	std::list<UIElement*> elements;
public:

	bool update(int width, int height){  // width and height are properties of main game window
		ImGui::NewFrame();
		
		bool ret = false;

		for(auto& element: elements)
			if(element->visible)
				ret = ret || element->update(width, height);
		

		ImGui::Render();
		for(auto& element: elements)
			if(element->expired){
				delete element;
				element = nullptr;
			}


		elements.remove_if([](UIElement* const& elem) -> bool { return elem == nullptr; });
		return ret;
	}

	UIElement* addNewElement(UIElement* elem){
		
		elements.push_back(elem);

		//TODO: add exception on NULL argument
		return elem;
	}

	void clear() {
		for(auto& element: elements)
			delete element;

		elements.resize(0);
	}

	~Manager() {
		clear();
	}

};

extern Manager manager;

};