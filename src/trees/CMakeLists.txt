set (FSM_TREES_SOURCES
	IOListContainer.cpp
	IOListContainer.h
	OutputTree.cpp
	OutputTree.h
	TestSuite.cpp
	TestSuite.h
	Tree.cpp
	Tree.h
	TreeEdge.cpp
	TreeEdge.h
	TreeNode.cpp
	TreeNode.h
)

add_library (fsm-trees ${FSM_TREES_SOURCES})
