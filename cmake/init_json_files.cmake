
# Initialize JSON files if they don't exist
if(NOT EXISTS "C:/Users/Fady2/Documents/DS_Project/project code/Data/users.json")
    file(WRITE "C:/Users/Fady2/Documents/DS_Project/project code/Data/users.json" "[]")
endif()

if(NOT EXISTS "C:/Users/Fady2/Documents/DS_Project/project code/Data/members.json")
    file(WRITE "C:/Users/Fady2/Documents/DS_Project/project code/Data/members.json" "[]")
endif()

if(NOT EXISTS "C:/Users/Fady2/Documents/DS_Project/project code/Data/classes.json")
    file(WRITE "C:/Users/Fady2/Documents/DS_Project/project code/Data/classes.json" "[]")
endif()

if(NOT EXISTS "C:/Users/Fady2/Documents/DS_Project/project code/Data/remembered.json")
    file(WRITE "C:/Users/Fady2/Documents/DS_Project/project code/Data/remembered.json" "{}")
endif()

# Copy existing files to build directory if they exist
if(EXISTS "C:/Users/Fady2/Documents/DS_Project/project code/Data/users.json")
    file(COPY "C:/Users/Fady2/Documents/DS_Project/project code/Data/users.json" DESTINATION "$<TARGET_FILE_DIR:DS_Project>/project code/Data")
endif()

if(EXISTS "C:/Users/Fady2/Documents/DS_Project/project code/Data/members.json")
    file(COPY "C:/Users/Fady2/Documents/DS_Project/project code/Data/members.json" DESTINATION "$<TARGET_FILE_DIR:DS_Project>/project code/Data")
endif()

if(EXISTS "C:/Users/Fady2/Documents/DS_Project/project code/Data/classes.json")
    file(COPY "C:/Users/Fady2/Documents/DS_Project/project code/Data/classes.json" DESTINATION "$<TARGET_FILE_DIR:DS_Project>/project code/Data")
endif()

if(EXISTS "C:/Users/Fady2/Documents/DS_Project/project code/Data/remembered.json")
    file(COPY "C:/Users/Fady2/Documents/DS_Project/project code/Data/remembered.json" DESTINATION "$<TARGET_FILE_DIR:DS_Project>/project code/Data")
endif()
