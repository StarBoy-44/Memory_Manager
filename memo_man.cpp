#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <iostream>
#include <vector>
#include <sstream>
#include <cmath>
#include <algorithm>

using namespace sf;
using namespace std;

struct Block {
    int id;
    bool allocated;
    bool freed;
    int memorySize;
    float glowAlpha;
    float scale;
    RectangleShape shape;
    Text idText;
    Text sizeText;

    Block(int _id, Font& font) : id(_id), allocated(false), freed(false), 
                                memorySize(0), glowAlpha(0.f), scale(1.f) {
        shape.setSize(Vector2f(60, 60)); // Larger blocks
        shape.setFillColor(Color(30, 40, 60,0)); // Darker blue
        shape.setOutlineThickness(2);
        shape.setOutlineColor(Color(50, 180, 255)); // Brighter neon blue
        
        idText.setFont(font);
        idText.setCharacterSize(14); // Slightly larger for visibility
        idText.setString(to_string(id));
        idText.setFillColor(Color(180, 220, 255));
        
        sizeText.setFont(font);
        sizeText.setCharacterSize(16); // Larger for better visibility
        sizeText.setFillColor(Color(240, 240, 255)); // Brighter white
        sizeText.setStyle(Text::Bold);
    }

    void update(float time) {
        glowAlpha = 60.f * (1.f + sin(time * 4.f + id * 0.15f));
        scale = allocated ? 1.f + 0.03f * sin(time * 4.f + id * 0.15f) : 1.f;
        
        if (allocated) {
            shape.setFillColor(Color(255, 90, 120)); // Brighter pink-red
        } else if (freed) {
            shape.setFillColor(Color(80, 255, 140)); // Brighter green
        } else {
            shape.setFillColor(Color(30, 40, 60)); // Dark blue
        }
        
        shape.setOutlineColor(Color(50, 180, 255, 255 - (int)glowAlpha));
    }

    void setPosition(Vector2f pos) {
        shape.setPosition(pos);
        
        // Position ID text centered above the block
        FloatRect idBounds = idText.getLocalBounds();
        idText.setOrigin(idBounds.width/2, idBounds.height);
        idText.setPosition(pos.x + shape.getSize().x/2, pos.y - 20);  // 10 pixels above
        
        // Position size text centered inside the block
        if (allocated) {
            sizeText.setString(to_string(memorySize) + "KB");
            FloatRect sizeBounds = sizeText.getLocalBounds();
            sizeText.setOrigin(sizeBounds.width/2, sizeBounds.height/2);
            sizeText.setPosition(pos.x + shape.getSize().x/2, pos.y + shape.getSize().y/2);
        }
    }
};

class MemoryUI {
    vector<Block> blocks;
    Font font;
    
    // UI Elements
    RectangleShape controlPanel, memoryPanel, summaryPanel;
    RectangleShape buttonAllocate, buttonFree, buttonClear, inputBox;
    Text titleText, statusText, summaryText, promptText;
    Text textAllocate, textFree, textClear;
    
    // Interactive state
    int selectedAction = 0;
    string userInputSize;
    bool showInputBox = false;
    int selectedBlock = -1;
    
    // Visual effects
    VertexArray backgroundGradient;
    VertexArray overlayGrid;
    vector<bool> buttonHover;
    vector<float> buttonGlow;
    Clock clock;
    float inputBoxGlowAlpha = 0.f;
    
    // Layout constants
    const Vector2f WINDOW_SIZE = Vector2f(1600, 800); // Larger window
    const Vector2i MEMORY_GRID_SIZE = Vector2i(10, 10);  // 16x6 grid (96 blocks)
    const Vector2f BLOCK_SIZE = Vector2f(50, 50);       // Larger blocks
    const float BLOCK_SPACING = 10.f;                   // More spacing

public:
    MemoryUI() {
        if (!font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf")) {
            cerr << "Failed to load font" << endl;
        }

        // Initialize blocks (96 total)
        for (int i = 0; i < 100; i++) {
            blocks.emplace_back(i + 1, font);
        }

        buttonHover = {false, false, false};
        buttonGlow = {0.f, 0.f, 0.f};

        setupUI();
    }

    void setupUI() {
        // Dark space-like background
        backgroundGradient = VertexArray(Quads, 4);
        backgroundGradient[0].position = Vector2f(0, 0);
        backgroundGradient[1].position = Vector2f(WINDOW_SIZE.x, 0);
        backgroundGradient[2].position = Vector2f(WINDOW_SIZE.x, WINDOW_SIZE.y);
        backgroundGradient[3].position = Vector2f(0, WINDOW_SIZE.y);
        backgroundGradient[0].color = Color(5, 10, 20);   // Dark space blue
        backgroundGradient[1].color = Color(5, 10, 20);
        backgroundGradient[2].color = Color(2, 5, 10);    // Even darker
        backgroundGradient[3].color = Color(2, 5, 10);

        // Subtle grid
        overlayGrid = VertexArray(Lines);
        Color gridColor(40, 90, 150, 20); // Very subtle blue
        for (int i = 0; i <= WINDOW_SIZE.x; i += 50) {
            overlayGrid.append(Vertex(Vector2f(i, 0), gridColor));
            overlayGrid.append(Vertex(Vector2f(i, WINDOW_SIZE.y), gridColor));
        }
        for (int i = 0; i <= WINDOW_SIZE.y; i += 50) {
            overlayGrid.append(Vertex(Vector2f(0, i), gridColor));
            overlayGrid.append(Vertex(Vector2f(WINDOW_SIZE.x, i), gridColor));
        }

        // Control panel (left)
        controlPanel.setSize(Vector2f(300, 900));
        controlPanel.setPosition(30, 30);
        controlPanel.setFillColor(Color(20, 30, 50, 220));
        controlPanel.setOutlineThickness(2);
        controlPanel.setOutlineColor(Color(60, 160, 255, 180));

        // Main memory display (center - much larger)
        memoryPanel.setSize(Vector2f(
            MEMORY_GRID_SIZE.x * ((BLOCK_SIZE.x + BLOCK_SPACING) * 1.7) + 30,
            MEMORY_GRID_SIZE.y * ((BLOCK_SIZE.y + BLOCK_SPACING) * 1.7) + 30
        ));
        memoryPanel.setPosition(
            controlPanel.getPosition().x + controlPanel.getSize().x + 30,
            30
        );
        memoryPanel.setFillColor(Color(15, 25, 40, 220));
        memoryPanel.setOutlineThickness(2);
        memoryPanel.setOutlineColor(Color(60, 160, 255, 180));

        // Summary panel (right)
        summaryPanel.setSize(Vector2f(300, 900));
        summaryPanel.setPosition(
            memoryPanel.getPosition().x + memoryPanel.getSize().x + 30,
            30
        );
        summaryPanel.setFillColor(Color(20, 30, 50, 220));
        summaryPanel.setOutlineThickness(2);
        summaryPanel.setOutlineColor(Color(60, 160, 255, 180));

        // Buttons
        setupButton(buttonAllocate, textAllocate, "Allocate", 70, 180);
        setupButton(buttonFree, textFree, "Free", 70, 280);
        setupButton(buttonClear, textClear, "Clear All", 70, 380);

        // Smaller title
        titleText.setFont(font);
        titleText.setString("Memory Visualizer");
        titleText.setCharacterSize(22); // Smaller size
        titleText.setFillColor(Color(120, 200, 255));
        titleText.setPosition(70, 70);
        titleText.setStyle(Text::Bold);

        statusText.setFont(font);
        statusText.setCharacterSize(18);
        statusText.setFillColor(Color(255, 120, 120));
        statusText.setPosition(70, 800);

        summaryText.setFont(font);
        summaryText.setCharacterSize(16);
        summaryText.setFillColor(Color(160, 220, 255));
        summaryText.setPosition(summaryPanel.getPosition().x + 20, 70);
        updateSummaryText();

        inputBox.setSize(Vector2f(300, 80));
        inputBox.setFillColor(Color(20, 30, 50, 220));
        inputBox.setOutlineThickness(2);
        inputBox.setOutlineColor(Color(60, 160, 255));

        promptText.setFont(font);
        promptText.setCharacterSize(18);
        promptText.setFillColor(Color(160, 220, 255));
        
        positionBlocks();
    }

    void positionBlocks() {
        Vector2f startPos(
            memoryPanel.getPosition().x + 20,
            memoryPanel.getPosition().y + 20
        );

        for (int i = 0; i < blocks.size(); ++i) {
            int row = i / MEMORY_GRID_SIZE.x;
            int col = i % MEMORY_GRID_SIZE.x;
            
            Vector2f blockPos(
                startPos.x + col * ((BLOCK_SIZE.x + BLOCK_SPACING) * 1.75),
                startPos.y + row * ((BLOCK_SIZE.y + BLOCK_SPACING) * 1.5)
            );
            
            blocks[i].setPosition(blockPos);
        }
    }

    void setupButton(RectangleShape& btn, Text& label, const string& name, float x, float y) {
        btn.setSize(Vector2f(180, 50));
        btn.setPosition(x, y);
        btn.setFillColor(Color(40, 70, 110));
        btn.setOutlineThickness(2);
        btn.setOutlineColor(Color(60, 160, 255));

        label.setFont(font);
        label.setString(name);
        label.setCharacterSize(20);
        label.setFillColor(Color(180, 220, 255));
        FloatRect textBounds = label.getLocalBounds();
        label.setOrigin(textBounds.width/2, textBounds.height/2);
        label.setPosition(x + btn.getSize().x/2, y + btn.getSize().y/2 - 2);
    }

    void updateSummaryText() {
        int allocatedCount = 0, freedCount = 0, totalSize = 0;
        int maxSize = 0, minSize = 100;
        vector<int> sizeHistogram(10, 0);

        for (const auto& b : blocks) {
            if (b.allocated) {
                allocatedCount++;
                totalSize += b.memorySize;
                maxSize = max(maxSize, b.memorySize);
                minSize = min(minSize, b.memorySize);
                
                int bin = (b.memorySize - 10) / 10;
                bin = max(0, min(9, bin));
                sizeHistogram[bin]++;
            } else if (b.freed) {
                freedCount++;
            }
        }

        stringstream ss;
        ss << "SYSTEM STATUS\n\n";
        ss << "Total Blocks: " << blocks.size() << "\n";
        ss << "Allocated: " << allocatedCount << "\n";
        ss << "Freed: " << freedCount << "\n";
        ss << "Available: " << (blocks.size() - allocatedCount) << "\n";
        ss << "Total Memory: " << totalSize << " KB\n";
        ss << "Max Block: " << (allocatedCount ? to_string(maxSize) : "N/A") << " KB\n";
        ss << "Min Block: " << (allocatedCount ? to_string(minSize) : "N/A") << " KB\n";
        ss << "Avg Block: " << (allocatedCount ? to_string(totalSize/allocatedCount) : "N/A") << " KB\n\n";
        
        ss << "MEMORY DISTRIBUTION\n";
        for (int i = 0; i < 10; i++) {
            int lower = 10 + i * 10;
            int upper = lower + 9;
            ss << lower << "-" << upper << " KB: " << string(sizeHistogram[i], '|') << "\n";
        }

        summaryText.setString(ss.str());
    }

    void handleMouseClick(Vector2f mousePos) {
        if (buttonAllocate.getGlobalBounds().contains(mousePos)) {
            selectedAction = 1;
            userInputSize.clear();
            showInputBox = true;
            statusText.setString("Select a block to allocate");
            positionInputBox();
            return;
        }
        
        if (buttonFree.getGlobalBounds().contains(mousePos)) {
            selectedAction = 2;
            showInputBox = false;
            statusText.setString("Select a block to free");
            return;
        }
        
        if (buttonClear.getGlobalBounds().contains(mousePos)) {
            for (auto& b : blocks) {
                b.allocated = false;
                b.freed = false;
                b.memorySize = 0;
            }
            selectedBlock = -1;
            statusText.setString("All blocks cleared");
            updateSummaryText();
            return;
        }

        for (int i = 0; i < blocks.size(); ++i) {
            if (blocks[i].shape.getGlobalBounds().contains(mousePos)) {
                selectedBlock = i;
                
                if (selectedAction == 1 && !blocks[i].allocated) {
                    showInputBox = true;
                    statusText.setString("Enter size for block " + to_string(i+1));
                    positionInputBox();
                } else if (selectedAction == 2 && blocks[i].allocated) {
                    blocks[i].allocated = false;
                    blocks[i].freed = true;
                    statusText.setString("Freed block " + to_string(i+1));
                    selectedAction = 0;
                    selectedBlock = -1;
                    updateSummaryText();
                }
                break;
            }
        }
    }

    void positionInputBox() {
        inputBox.setPosition(
            memoryPanel.getPosition().x + (memoryPanel.getSize().x - inputBox.getSize().x)/2,
            memoryPanel.getPosition().y + (memoryPanel.getSize().y - inputBox.getSize().y)/2
        );
        
        promptText.setPosition(
            inputBox.getPosition().x + 20,
            inputBox.getPosition().y + 20
        );
    }

    void handleMouseMove(Vector2f mousePos) {
        buttonHover[0] = buttonAllocate.getGlobalBounds().contains(mousePos);
        buttonHover[1] = buttonFree.getGlobalBounds().contains(mousePos);
        buttonHover[2] = buttonClear.getGlobalBounds().contains(mousePos);
    }

    void handleTextInput(Event& e) {
        if (e.text.unicode == '\b') {
            if (!userInputSize.empty()) userInputSize.pop_back();
        } else if (e.text.unicode == '\r') {
            applyAction();
        } else if (isdigit(e.text.unicode)) {
            if (userInputSize.size() < 3) {
                userInputSize += static_cast<char>(e.text.unicode);
            }
        }
    }

    void applyAction() {
        if (selectedBlock == -1 || userInputSize.empty()) return;

        int size = stoi(userInputSize);
        if (size < 10 || size > 100) {
            statusText.setString("Size must be 10-100 KB");
            return;
        }
        
        if (blocks[selectedBlock].allocated) {
            statusText.setString("Block already allocated");
            return;
        }

        blocks[selectedBlock].allocated = true;
        blocks[selectedBlock].freed = false;
        blocks[selectedBlock].memorySize = size;
        
        statusText.setString("Allocated block " + to_string(selectedBlock+1) + 
                            " (" + to_string(size) + " KB)");
        
        selectedAction = 0;
        showInputBox = false;
        selectedBlock = -1;
        userInputSize.clear();
        updateSummaryText();
    }

    void draw(RenderWindow& window) {
        float time = clock.getElapsedTime().asSeconds();

        inputBoxGlowAlpha = 60.f * (1.f + sin(time * 4.f));
        for (int i = 0; i < 3; ++i) {
            buttonGlow[i] = buttonHover[i] ? 60.f * (1.f + sin(time * 4.f)) : 0.f;
        }

        window.draw(backgroundGradient);
        window.draw(overlayGrid);

        window.draw(controlPanel);
        window.draw(memoryPanel);
        window.draw(summaryPanel);

        titleText.setFillColor(Color(
            120 + (int)(30 * sin(time * 2.f)),
            200 + (int)(30 * sin(time * 2.f + 1.f)),
            255
        ));
        window.draw(titleText);

        updateButton(buttonAllocate, textAllocate, buttonGlow[0]);
        updateButton(buttonFree, textFree, buttonGlow[1]);
        updateButton(buttonClear, textClear, buttonGlow[2]);
        
        window.draw(buttonAllocate); window.draw(textAllocate);
        window.draw(buttonFree); window.draw(textFree);
        window.draw(buttonClear); window.draw(textClear);

        // Draw all IDs first (so they appear behind if blocks overlap)
        
        // Then draw all blocks
        for (auto& block : blocks) {
            block.update(time);
            
            if (&block == &blocks[selectedBlock]) {
                block.shape.setOutlineColor(Color(220, 220, 255));
                block.shape.setOutlineThickness(4);
            } else {
                block.shape.setOutlineThickness(2);
            }
            
            window.draw(block.shape);
        }
        
        for (auto& block : blocks) {
            window.draw(block.idText);
        }

        // Finally draw all size texts (on top of blocks)
        for (auto& block : blocks) {
            if (block.allocated) {
                window.draw(block.sizeText);
            }
        }

        if (showInputBox) {
            inputBox.setOutlineColor(Color(
                60, 160, 255, 
                255 - (int)inputBoxGlowAlpha
            ));
            window.draw(inputBox);

            promptText.setString(
                selectedBlock == -1 ? "Select a block first" : 
                "Enter Size (10-100 KB):\n" + userInputSize
            );
            
            promptText.setFillColor(Color(
                160 + (int)(50 * sin(time * 3.f)),
                220 + (int)(35 * sin(time * 3.f + 1.f)),
                255
            ));
            
            window.draw(promptText);
        }

        statusText.setFillColor(Color(
            255, 
            120 + (int)(50 * sin(time * 2.f)),
            120 + (int)(50 * sin(time * 2.f + 1.f))
        ));
        window.draw(statusText);
        window.draw(summaryText);
    }

    void updateButton(RectangleShape& btn, Text& text, float glow) {
        btn.setOutlineColor(Color(60, 160, 255, 255 - (int)glow));
        text.setFillColor(Color(
            180 + (int)(glow/3),
            220 + (int)(glow/3),
            255
        ));
    }

    void update(RenderWindow& window, Event& event) {
        if (event.type == Event::MouseButtonPressed) {
            handleMouseClick(window.mapPixelToCoords(Mouse::getPosition(window)));
        }
        
        if (event.type == Event::MouseMoved) {
            handleMouseMove(window.mapPixelToCoords(Mouse::getPosition(window)));
        }
        
        if (showInputBox && event.type == Event::TextEntered) {
            handleTextInput(event);
        }
    }
};

int main() {
    RenderWindow window(VideoMode(1800, 1000), "Memory Visualizer");
    window.setFramerateLimit(60);
    
    MemoryUI memoryUI;

    while (window.isOpen()) {
        Event event;
        while (window.pollEvent(event)) {
            if (event.type == Event::Closed) {
                window.close();
            }
            memoryUI.update(window, event);
        }

        window.clear();
        memoryUI.draw(window);
        window.display();
    }

    return 0;
}