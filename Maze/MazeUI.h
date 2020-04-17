#pragma once
#include "../external/imgui/imgui.h"
#include <string>
#include <sstream>
#include <iostream>



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
	virtual void update(int width, int height) = 0;

	virtual ~UIElement() {};
};







class WindowItem {
/*
	

	Base interface for any widget drawn on ImGui Window



*/
public:
	virtual void update() = 0;

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
	
	void update() override{
		ImGui::TextUnformatted(text.c_str());
	}

};

template<typename Stat>
class StatText: public Text {
/*


	Text lables that diplay some information in next form: "STAT_NAME: STAT"


*/
	Stat* stat_;
	std::string stat_name_;
public:
	StatText(Stat* stat, std::string const& stat_name = "Stat"): stat_(stat), stat_name_(stat_name){
		//TODO add exception on NULL stat
		std::ostringstream oss;
		oss << stat_name_ << ": " << *stat_;
		text = oss.str();
	}
	void update() override{
		std::ostringstream oss;
		oss << stat_name_ << ": " << *stat_;
		text = oss.str();
		Text::update();
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

	void update() override{
		if(ImGui::Button(label.c_str(), ImVec2(xSize, ySize)))
			onClick();

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

	virtual void updElems(){
		for(auto& item: items)
			item->update();
	};

public:

	Window(std::string const& head = "Window", float xP = 0.0f, float yP = 0.0f, float xW = 0.1f, float yW = 0.1f, int fl = 0): 
	header(head), xPos(xP), yPos(yP), xWide(xW), yWide(yW), flags(static_cast<enum ImGuiWindowFlags_>(fl))
	{}

	void update(int width, int height) override{
		
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
		ImGui::SetNextWindowPos(ImVec2(static_cast<float>(width) * xPos, static_cast<float>(height) * yPos));
		ImGui::SetNextWindowSize(ImVec2(static_cast<float>(width) * xWide, static_cast<float>(height) * yWide), ImGuiCond_Always);
		ImGui::Begin(header.c_str(), nullptr, flags);
		updElems();
		ImGui::End();
		ImGui::PopStyleVar();

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


class Manager final{
/*
	

	Class that Owns all UIElements 


*/
	std::vector<UIElement*> elements;
public:

	void update(int width, int height){  // width and height are properties of main game window
		ImGui::NewFrame();
		
		for(auto& element: elements)
			element->update(width, height);
		

		ImGui::Render();
	}

	void addNewElement(UIElement* elem){
		
		elements.push_back(elem);

		//TODO: add exception on NULL argument
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