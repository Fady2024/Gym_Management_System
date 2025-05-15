inline QString clockStyle = R"(
QWidget {
    background-color: rgba(139, 92, 246, 0.05); /* Soft lavender */
    border-radius: 12px;
    padding: 0px;
    border: 1px solid #BFAEF5; /* Muted lavender */
}
)";

inline QString clockLabelStyle = R"(
background-color: transparent;
color: rgba(139, 92, 246, 0.9);
font-size: 17px;
border-radius: 12px;
font-weight: bold;
)";

inline QString clockButtonStyle = R"(
QPushButton {
    background-color: transparent;
    color: rgba(191, 174, 245, 0.8);
    border: none;
    font-size: 15px;
    border-radius: 12px;
}
QPushButton:hover {
    background-color: rgba(98, 38, 182, 0.15);
    color: rgba(98, 38, 182, 1);
}
)";
