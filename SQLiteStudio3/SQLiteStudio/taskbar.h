#ifndef TASKBAR_H
#define TASKBAR_H

#include <QToolBar>
#include <QActionGroup>

class QMimeData;
class QToolButton;
class QRubberBand;

// TODO enclose task<->mdiWindow relation inside a task class and make it managed by taskbar, not by mdiarea
class TaskBar : public QToolBar
{
        Q_OBJECT
    public:
        TaskBar(const QString& title, QWidget *parent = 0);
        explicit TaskBar(QWidget *parent = 0);

        QAction* addTask(const QIcon& icon, const QString& text);
        void removeTask(QAction* action);
        QList<QAction*> getTasks() const;
        bool isEmpty();
        int count();

    protected:
        void mousePressEvent(QMouseEvent* event);
        void mouseMoveEvent(QMouseEvent* event);
        void dragEnterEvent(QDragEnterEvent* event);
        void dragMoveEvent(QDragMoveEvent* event);
        void dropEvent(QDropEvent* event);

    private:
        void init();
        bool handleMouseMoveEvent(QMouseEvent* event);
        QToolButton* getToolButton(QAction* action);
        QAction* getNextClosestAction(const QPoint& position);
        void dragTaskTo(QAction* task, const QPoint& position);
        void dragTaskTo(QAction* task, int positionIndex);
        QAction* getDragTask(const QMimeData* data);
        QMimeData* generateMimeData();
        int getActiveTaskIdx();

        constexpr static const char* mimeDataId = "application/x-sqlitestudio-taskbar-task";

        /**
         * @brief getDropPositionIndex
         * @param task
         * @param position
         * @return Index of action in actions() that drag should be inserting dropped item just before, or -1 to indicate "at the end".
         */
        int getDropPositionIndex(QAction* task, const QPoint& position);

        QActionGroup taskGroup;
        QList<QAction*> tasks;
        QAction* dragStartTask = nullptr;
        QPoint dragStartPosition;
        int dragStartIndex;
        int dragCurrentIndex;
        QMenu* taskMenu = nullptr;
        QAction* closeSelectedAction = nullptr;
        QAction* closeAllButSelectedAction = nullptr;
        QAction* closeAllAction = nullptr;
        QAction* restoreLastClosedAction = nullptr;
        QAction* renameSelectedAction = nullptr;

    public slots:
        void nextTask();
        void prevTask();

    private slots:
        void taskBarMenuRequested(const QPoint& p);
        void mousePressed();
        void closeAllTasks();
        void closeAllTasksButSelected();
        void closeSelectedTask();
        void restoreLastClosedTask();
        void renameSelectedTask();
};

#endif // TASKBAR_H
