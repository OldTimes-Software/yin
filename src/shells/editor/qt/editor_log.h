
#pragma once

#include <plcore/pl_math.h>

#include <QPlainTextEdit>

namespace os
{
	class EditorLog : public QPlainTextEdit
	{
	public:
		EditorLog();

		void PushMessage( int level, const char *msg, PLColour colour );

	private:
		EditorLog *instance{ nullptr };
	};
}// namespace os
