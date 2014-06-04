// Copyright 2014 Citra Emulator Project
// Licensed under GPLv2
// Refer to the license.txt file included.

#include "graphics.hxx"
#include <QListView>
#include <QVBoxLayout>
#include <QDebug>

extern GraphicsDebugger g_debugger;

GPUCommandStreamItemModel::GPUCommandStreamItemModel(QObject* parent) : QAbstractListModel(parent), command_count(0)
{
    connect(this, SIGNAL(GXCommandFinished(int)), this, SLOT(OnGXCommandFinishedInternal(int)));
}

int GPUCommandStreamItemModel::rowCount(const QModelIndex& parent) const
{
    return command_count;
}

QVariant GPUCommandStreamItemModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    int command_index = index.row();
    const GSP_GPU::GXCommand& command = GetDebugger()->ReadGXCommandHistory(command_index);
    if (role == Qt::DisplayRole)
    {
        std::map<GSP_GPU::GXCommandId, const char*> command_names;
        command_names[GSP_GPU::GXCommandId::REQUEST_DMA] = "REQUEST_DMA";
        command_names[GSP_GPU::GXCommandId::SET_COMMAND_LIST_FIRST] = "SET_COMMAND_LIST_FIRST";
        command_names[GSP_GPU::GXCommandId::SET_MEMORY_FILL] = "SET_MEMORY_FILL";
        command_names[GSP_GPU::GXCommandId::SET_DISPLAY_TRANSFER] = "SET_DISPLAY_TRANSFER";
        command_names[GSP_GPU::GXCommandId::SET_TEXTURE_COPY] = "SET_TEXTURE_COPY";
        command_names[GSP_GPU::GXCommandId::SET_COMMAND_LIST_LAST] = "SET_COMMAND_LIST_LAST";
        QString str = QString("%1 %2 %3 %4 %5 %6 %7 %8 %9").arg(command_names[static_cast<GSP_GPU::GXCommandId>(command.id)])
                        .arg(command.data[0], 8, 16, QLatin1Char('0'))
                        .arg(command.data[1], 8, 16, QLatin1Char('0'))
                        .arg(command.data[2], 8, 16, QLatin1Char('0'))
                        .arg(command.data[3], 8, 16, QLatin1Char('0'))
                        .arg(command.data[4], 8, 16, QLatin1Char('0'))
                        .arg(command.data[5], 8, 16, QLatin1Char('0'))
                        .arg(command.data[6], 8, 16, QLatin1Char('0'))
                        .arg(command.data[7], 8, 16, QLatin1Char('0'));
        return QVariant(str);
    }
    else
    {
        return QVariant();
    }
}

void GPUCommandStreamItemModel::GXCommandProcessed(int total_command_count)
{
    emit GXCommandFinished(total_command_count);
}

void GPUCommandStreamItemModel::OnGXCommandFinishedInternal(int total_command_count)
{
    if (total_command_count == 0)
        return;

    int prev_command_count = command_count;
    command_count = total_command_count;
    emit dataChanged(index(prev_command_count,0), index(total_command_count-1,0));
}


GPUCommandStreamWidget::GPUCommandStreamWidget(QWidget* parent) : QDockWidget(tr("Graphics Debugger"), parent)
{
    // TODO: set objectName!

    GPUCommandStreamItemModel* command_model = new GPUCommandStreamItemModel(this);
    g_debugger.RegisterObserver(command_model);

    QListView* command_list = new QListView;
    command_list->setModel(command_model);
    command_list->setFont(QFont("monospace"));

    setWidget(command_list);
}
