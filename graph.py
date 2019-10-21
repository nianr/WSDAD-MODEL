import csv
import matplotlib.pyplot as plt
from matplotlib.backends.backend_pdf import PdfPages
import os
import csv


def online(name, a):
    a = list(a)
    data_new_linear = []
    data_new_log = []
    data_linear = []
    data_log = []
    basellne = []
    data = []
        path = "./test/outtest/"+name+"/"
    for i in range(len(a)):
        s = path + a[i] + ".csv"
        if os.path.exists(s):
            file = csv.reader(open(s,"r"))
            for row in file:
                if row[0] == '1':
                    data.append(row)
    for i in range(len(data)):
        data[i].pop(0)
        if not i == len(data)-1:
            data[i].pop(-1)
        for j in range(len(data[i])):
            data[i][j] = int(data[i][j])
    m = 0
    for i in range(len(data)):
        for j in range(len(data[i])):
            data[i][j] = data[i][j] + m
        m = data[i][-1]
    for i in range(len(data)):
        for j in range(len(data[i])):
            if k == 0:
                data_new_linear.append(data[i][j])
            else:
                data_new_log.append(data[i][j])

#    file1 = csv.reader(open("/Users/chenyifan/Downloads/FeedbackIsolationForest-master/test/"
#                           "outtest/"+name+"/"+name+"_online_linear.csv", "r"))
#    file2 = csv.reader(open("/Users/chenyifan/Downloads/FeedbackIsolationForest-master/test/"
#                           "outtest/"+name+"/"+name+"_online_log.csv", "r"))
#    file3 = csv.reader(open("/Users/chenyifan/Downloads/FeedbackIsolationForest-master/test/"
#                           "outtest/"+name+"/"+name+"_online_baseline.csv", "r"))
#    for row in file1:
#        if row[0] == '1':
#            data = row
#    data.pop(0)
#    for i in range(len(data)):
#        data[i] = int(data[i])
#    data_linear = data
#
#    for row in file2:
#        if row[0] == '1':
#            data = row
#    data.pop(0)
#    for i in range(len(data)):
#        data[i] = int(data[i])
#    data_log = data
#
#    for row in file3:
#        if row[0] == '1':
#            data = row
#    data.pop(0)
#    for i in range(len(data)):
#        data[i] = int(data[i])
#    baseline = data
    # file = open("/Users/chenyifan/Documents/results.txt", "a+")
    # file.write(str(data_new_linear[-1] / data_linear[-1]) + "\n")

    while len(data_new_linear) < 150:
        data_new_linear.append(data_new_linear[-1])
#    while len(data_new_log) < 150:
#        data_new_log.append(data_new_log[-1])

    w_new_linear, w_new_log, w_linear, w_log, w_baseline = [], [], [], [], []
    writer = csv.writer(open("/Users/chenyifan/Documents/WSDAD/data/"+name+"_online.csv", "a+"))
    # writer = csv.writer(open("/Users/chenyifan/Documents/WSDAD/data/julei.csv", "a+"))
    w_new_linear.append(data_new_linear[0])
#    w_new_log.append(data_new_log[0])
#    w_linear.append(data_linear[0])
#    w_log.append(data_log[0])
#    w_baseline.append(baseline[0])
    for i in range(1, 16):
        w_new_linear.append(data_new_linear[i*10-1])
#        w_new_log.append(data_new_log[i*10-1])
#        w_linear.append(data_linear[i*10-1])
#        w_log.append(data_log[i*10-1])
#        w_baseline.append(baseline[i*10-1])
        # w_linear.append(data_s[i* 10])

    writer.writerow(w_new_linear)
#    writer.writerow(w_new_log)
#    writer.writerow(w_linear)
#    writer.writerow(w_log)
#    writer.writerow(w_baseline)

    # pdf = PdfPages("/Users/chenyifan/Documents/unfinished/"+name+"_online.pdf")
    plt.grid(axis="y")
    plt.plot(range(len(data_new_linear)), data_new_linear, mfc='w', label='WSDAD_linear', markevery=80)
#    plt.plot(range(len(data_new_log)),  data_new_log, mfc='w', label='WSDAD_log', markevery=80)
#    plt.plot(range(len(data_linear)),  data_linear, mfc='w', label='FGAD_linear', markevery=80)
#    plt.plot(range(len(data_log)),  data_log, mfc='w', label='FGAD_log', markevery=80)
#    plt.plot(range(len(baseline)), baseline, mfc='w', label='Baseline', markevery=80)
    plt.legend(loc=0, ncol=1)
    plt.xlabel('Feedback')
    plt.ylabel('Anomaly')
    # pdf.savefig()
    # pdf.close()
    plt.show()


def batch(name, a):
    a = list(a)
    data_new_linear = []
    data_new_log = []
    data_linear = []
    data_log = []
    basellne = []
    for k in range(2):
        data = []
        if k == 0:
            path = "/Users/chenyifan/Documents/GitHub/FeedbackIsolationForest/test/outtest/"+name+"/batch/linear/"
        else:
            path = "/Users/chenyifan/Documents/GitHub/FeedbackIsolationForest/test/outtest/"+name+"/batch/log/"
        for i in range(len(a)):
            s = path + a[i] + ".csv"
            if os.path.exists(s):
                file = csv.reader(open(s, "r"))
                for row in file:
                    if row[0] == '1':
                        data.append(row)
        for i in range(len(data)):
            data[i].pop(0)
            if not i == len(data)-1:
                data[i].pop(-1)
            for j in range(len(data[i])):
                data[i][j] = int(data[i][j])
        m = 0
        for i in range(len(data)):
            for j in range(len(data[i])):
                data[i][j] = data[i][j] + m
            m = data[i][-1]
        for i in range(len(data)):
            for j in range(len(data[i])):
                if k == 0:
                    data_new_linear.append(data[i][j])
                else:
                    data_new_log.append(data[i][j])

    file1 = csv.reader(open("/Users/chenyifan/Downloads/FeedbackIsolationForest-master/test/"
                           "outtest/"+name+"/"+name+"_batch_linear.csv", "r"))
    file2 = csv.reader(open("/Users/chenyifan/Downloads/FeedbackIsolationForest-master/test/"
                           "outtest/"+name+"/"+name+"_batch_log.csv", "r"))
    file3 = csv.reader(open("/Users/chenyifan/Downloads/FeedbackIsolationForest-master/test/"
                           "outtest/"+name+"/"+name+"_batch_baseline.csv", "r"))
    for row in file1:
        if row[0] == '1':
            data = row
    data.pop(0)
    for i in range(len(data)):
        data[i] = int(data[i])
    data_linear = data

    for row in file2:
        if row[0] == '1':
            data = row
    data.pop(0)
    for i in range(len(data)):
        data[i] = int(data[i])
    data_log = data

    for row in file3:
        if row[0] == '1':
            data = row
    data.pop(0)
    for i in range(len(data)):
        data[i] = int(data[i])
    baseline = data
    file = open("/Users/chenyifan/Documents/results.txt", "a+")
    file.write(str(data_new_linear[-1] / data_linear[-1]) + "\n")

    while len(data_new_linear) < 150:
        data_new_linear.append(data_new_linear[-1])
    while len(data_new_log) < 150:
        data_new_log.append(data_new_log[-1])

    w_new_linear, w_new_log, w_linear, w_log, w_baseline = [], [], [], [], []
    writer = csv.writer(open("/Users/chenyifan/Documents/WSDAD/data/" + name + "_batch.csv", "a+"))
    w_new_linear.append(data_new_linear[0])
    w_new_log.append(data_new_log[0])
    w_linear.append(data_linear[0])
    w_log.append(data_log[0])
    w_baseline.append(baseline[0])
    for i in range(1, 16):
        w_new_linear.append(data_new_linear[i * 10 - 1])
        w_new_log.append(data_new_log[i * 10 - 1])
        w_linear.append(data_linear[i * 10 - 1])
        w_log.append(data_log[i * 10 - 1])
        w_baseline.append(baseline[i*10 - 1])

    writer.writerow(w_new_linear)
    writer.writerow(w_new_log)
    writer.writerow(w_linear)
    writer.writerow(w_log)
    writer.writerow(w_baseline)

    # pdf = PdfPages("/Users/chenyifan/Documents/unfinished/"+name+"_batch.pdf")
    plt.grid(axis="y")
    plt.plot(range(len(data_new_linear)), data_new_linear, mfc='w', label='WSDAD_linear', markevery=80)
    plt.plot(range(len(data_new_log)),  data_new_log, mfc='w', label='WSDAD_log', markevery=80)
    plt.plot(range(len(data_linear)),  data_linear, mfc='w', label='FGAD_linear', markevery=80)
    plt.plot(range(len(data_log)),  data_log, mfc='w', label='FGAD_log', markevery=80)
    plt.plot(range(len(baseline)), baseline, mfc='w', label='Baseline', markevery=80)
    plt.legend(loc=0, ncol=1)
    plt.xlabel('Feedback')
    plt.ylabel('Anomaly')
    # pdf.savefig()
    # pdf.close()
    plt.show()


def barh():
    data = []
    colors = []
    file = open("/Users/chenyifan/Documents/WSDAD/results.txt", "r")
    for row in file:
        if row != '\n':
            data.append(float(row)-1)
    # print(data)
    data.pop(2)
    data.pop(2)
    data.pop(-1)
    data.pop(-1)
    data.pop(14)
    data.pop(14)
    name = ["abalone_online", "abalone_batch",
            #"ann_online", "ann_batch",
            "card_online", "card_batch",
            "covtype_online", "covtype_batch",
            "covtype_sub_online", "covtype_sub_batch",
            "kddcup_online", "kddcup_batch",
            "kddcup_sub_online", "kddcup_sub_batch",
            "mam_online", "mam_batch",
            #"mam_sub_online", "mam_sub_batch",
            "shuttle_online", "shuttle_batch",
            "shuttle_sub_online", "shuttle_sub_batch",
            "toy_online", "toy_batch",
            "toy2_online", "toy2_batch"]
            #"yeast_online", "yeast_batch"]
    for item in data:
        if item < 0:
            colors.append('red')
        else:
            colors.append('dodgerblue')
    plt.barh(name, data, color=colors)
    plt.xlabel('Rate')
    plt.ylabel('Dataset')
    plt.tight_layout()
    pdf = PdfPages("/Users/chenyifan/Documents/result_v2.pdf")
    pdf.savefig(dpi=100)
    pdf.close()
    plt.show()


def julei():
    # data_s = []
    # for i in range(len(data_new_log)):
    #     if data_new_log[i] > 3:
    #         data_new_log[i] = data_new_log[i]-2
    #         data_s.append(abs(data_new_log[i]-1))
    # plt.plot(range(len(data_new_linear)), data_new_linear, mfc='w', label='K-means', markevery=80)
    # plt.plot(range(len(data_new_log)), data_new_log, mfc='w', label='mean-shift', markevery=80)
    # plt.plot(range(len(data_s)), data_s, mfc='w', label='DBSCAN', markevery=80)
    online("abalone", "0621374985")


def ttt():
    a = "0123456789"
    data_new_linear = []
    data = []
    path = "/Users/chenyifan/Documents/GitHub/FeedbackIsolationForest/test/outtest/kddcup_-/"
    for i in range(len(a)):
        s = path + a[i] + ".csv"
        if os.path.exists(s):
            file = csv.reader(open(s, "r"))
            for row in file:
                print(row)
                if row[0] == '1':
                    data.append(row)
    for i in range(len(data)):
        data[i].pop(0)
        if not i == len(data) - 1:
            data[i].pop(-1)
        for j in range(len(data[i])):
            data[i][j] = int(data[i][j])
    m = 0
    for i in range(len(data)):
        for j in range(len(data[i])):
            data[i][j] = data[i][j] + m
        m = data[i][-1]
    for i in range(len(data)):
        for j in range(len(data[i])):
            data_new_linear.append(data[i][j])
    print(data_new_linear)
    while len(data_new_linear) < 150:
        data_new_linear.append(data_new_linear[-1])
    writer = csv.writer(open("/Users/chenyifan/Documents/WSDAD/kddcup_-.csv", "a+"))
    w_new_linear, w_new_log, w_linear, w_log, w_baseline = [], [], [], [], []
    for i in range(1, 16):
        w_new_linear.append(data_new_linear[i*10-1])
    # print(data_new_linear[0])
    writer.writerow(w_new_linear)


if __name__ == '__main__':
    online("abalone", "0621374985")
    # batch("abalone", "0621374985")
    # online("ann", "0391247")
    # batch("ann", "0391247586")
    # online("card", "6702398145")
    # batch("card", "6702398145")
    # online("covtype", "5862074139")
    # batch("covtype", "586207")
    # online("covtype_sub", "6509278341")
    # batch("covtype_sub", "6509278341")
    # online("kddcup", "6")
    # batch("kddcup", "6")
    # online("kddcup_sub", "5708962314")
    # batch("kddcup_sub", "5708962314")
    # online("mam", "0")
    # batch("mam", "0")
    # online("mam_sub", "9240813576")
    # batch("mam_sub", "9240813576")
    # online("shuttle", "5")
    # batch("shuttle", "5")
    # online("shuttle_sub", "8706")
    # batch("shuttle_sub", "8706")
    # online("toy", "1769538042")
    # batch("toy", "1769538042")
    # online("toy2", "2376108459")
    # batch("toy2", "2376108459")
    # online("yeast", "2371694805")
    # batch("yeast", "2371694805")
    # barh()
    # julei()
    # ttt()
