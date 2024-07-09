#!/usr/bin/env python

import sys
import re

def find_intersect (coordinates):
    #[[x1,y1],[x2,y2]],[[x3,y3],[x4,y4]]
    x1, y1 = coordinates[0][0][0], coordinates[0][0][1]
    x2, y2 = coordinates[0][1][0], coordinates[0][1][1]
    x3, y3 = coordinates[1][0][0], coordinates[1][0][1]
    x4, y4 = coordinates[1][1][0], coordinates[1][1][1]
 
    try:
        m1 = (y1-y2)/(x1-x2)  # Pay attention to not dividing by zero
        m1 = round(m1,4)
    except:
        m1  = 'Nan'

    try:
        m2 = (y3-y4)/(x3-x4)  # Pay attention to not dividing by zero
        m2 = round(m2,4)
    except:
        m2 = 'Nan'

    if (m1 == m2):
        return [False, None, None]  # Parallel segments
    
    xnum = ((x1*y2-y1*x2)*(x3-x4) - (x1-x2)*(x3*y4-y3*x4))
    xden = ((x1-x2)*(y3-y4) - (y1-y2)*(x3-x4))
    xcoor = round( xnum / xden, 4)

    ynum = (x1*y2 - y1*x2)*(y3-y4) - (y1-y2)*(x3*y4-y3*x4)
    yden = (x1-x2)*(y3-y4) - (y1-y2)*(x3-x4)
    ycoor = round( ynum / yden, 4)
    
    if ((((xcoor >= x1 and xcoor <= x2) or (xcoor <= x1 and xcoor >= x2)) and ((xcoor >= x3 and xcoor <= x4) or (xcoor <= x3 and xcoor >= x4))) and
       (((ycoor >= y1 and ycoor <= y2) or (ycoor <= y1 and ycoor >= y2)) and ((ycoor >= y3 and ycoor <= y4) or (ycoor <= y3 and ycoor >= y4)))):
        intersection = True;
    else:
        return [False, None, None]  # intersection is out of bounds
    
    return [intersection, xcoor, ycoor]   

def parseLine(line):

    vertices = [];

    #strip the line and remove the white spaces
    sp = line.strip().split();

    #input validation
    if len(sp) == 0:
        raise Exception('Command must not be blank.');
        return NULL;
    elif len(sp) == 1:
        if sp[0] =='gg':
            cmd = 4;
            return [cmd, 0, 0];
        else:
            raise Exception('Command provide is invalid')
            return NULL;
    else:
        #extract the command  
        if sp[0] =='add':
            cmd = 1;
        elif sp[0] =='mod':
            cmd = 2;
        elif sp[0] =='rm':
            cmd = 3;
        else:
            raise Exception('Command provided is invalid');
            return NULL
        
        #extract the street name
        pattern = '"[^"]*"';
        regex_result = re.search(pattern, line);
        if(regex_result):
            stname = line[regex_result.span()[0]:regex_result.span()[1]];
        else:
            raise Exception('No valid street name found. Provide proper street name in double quotes')
            return NULL;

        if stname == "\"\"":
            raise Exception('Street name cannot be empty')
            return NULL;   
             
        #lets remove the cmd and street name part from the line
        line = line[regex_result.span()[1]+1:]
        sp = line.strip().split(")");

        #if it's a rm command, make sure there's nothing else left in the line, and then let's exit here, we have everything we need
        if (cmd == 3) :
            if line.replace(" ", "") == '\n' or line.replace(" ", "") == '':
                return [cmd,stname,0]; 
            else:
                raise Exception('Invalid rm command');
                return NULL


        for i in sp:
            pattern_xpos_ypos = '\([0-9]+,[0-9]+\)'; #detects (x,y)
            pattern_xpos_yneg = '\([0-9]+,-[0-9]+\)'; #detects (x,-y)
            pattern_xneg_ypos = '\(-[0-9]+,[0-9]+\)'; #detects (-x,y)
            pattern_xneg_yneg = '\(-[0-9]+,-[0-9]+\)'; #detects (-x,-y)
            
            #strip if any blank spaces are present
            i = i.replace(" ", "")
            i = i + ')'
            if i == ')':
                break;

            #caretestin coordinates must be provided in one of the four above forms, if else raise an exception
            if re.match(pattern_xpos_ypos, i) or re.match(pattern_xpos_yneg, i) or re.match(pattern_xneg_ypos, i) or re.match(pattern_xneg_yneg, i):
                #Find , 
                comma_pos = re.search(',',i).start();
                #Find y coordinate
                x = int(i[1:comma_pos]);
                #Find y coordinate
                y = int(i[comma_pos+1:-1]);

                vertices.append ([float(x),float(y)]);         
            else:
                raise Exception('Invalid Cartesian coordinates. Cartestian coordinates must be provided in (x,y) format');
                return NULL

        if sp[-1].replace(" ","") != '':
            raise Exception('Invalid Cartesian coordinates. Cartestian coordinates must be provided in (x,y) format');
            return NULL

        if (cmd == 1 or cmd == 2) and len(vertices) <2 :
            raise Exception('Atleast two coordinates required for add and mod commands');
            return NULL 

    return [cmd,stname,vertices]; 

class Minimum_Traffic_Lights(object):

    master_list = [];
    vertex = [];
    edges = []; 
    vertex_num = 0;
    
    def get(self):
        return self.v
    
    def add(self, streetname, raw_vertices):
        for i in self.master_list:
            if i[0] == streetname:
                raise Exception('The street name already exists');
        self.master_list.append([streetname, raw_vertices]);
    
    def mod(self, streetname, raw_vertices):
        st_name_match = False;
        for i in self.master_list:
            if i[0] == streetname:
                self.master_list.remove(i);
                st_name_match = True;
             
        if (st_name_match == True):
            self.master_list.append([streetname, raw_vertices]);
            st_name_match = False;
        else:
            raise Exception('mod specified for a street that does not exist');

    def rm(self, streetname):
        st_name_match = False;
        for i in self.master_list:
            if i[0] == streetname:
                self.master_list.remove(i);
                st_name_match = True;

        if (st_name_match == False):
            raise Exception('rm specified for a street that does not exist');
             
    def gg(self):
        #step 0 : Clear the vertices and edges list
        self.vertex.clear();
        self.edges.clear();
        self.vertex_num = 0;
        #step 1 : Parse the master list and generate the vertices and edges list.
        for i in self.master_list:
             past_xy = None;
             curr_xy =None;
             for j in i[1]:

                #check if the new vertex is already in vertex list, if it is don't add again
                vertex_found = False;
                vertex_used = 0
                for k in self.vertex:
                    if k[1] == j:
                        vertex_found = True;
                        vertex_used = k[0]

                if vertex_found is False:
                    self.vertex_num = self.vertex_num + 1;
                    self.vertex.append([self.vertex_num,j]);                   
                    vertex_used = self.vertex_num 
                else:
                    pass
                
                #However calculate edge as usual
                if curr_xy is None:
                    curr_xy = [vertex_used, j];
                else:
                    past_xy = curr_xy;
                    curr_xy = [vertex_used,j];
                    if (curr_xy[1] != past_xy[1]): 
                        #Dont add an edge if the edge is 1 dimensional
                        self.edges.append([past_xy,curr_xy])
            
        #step 3: Detect intersections

        intersection_free = False;
    
        i_start_index = 0
        while (intersection_free is False):
            intersection_free = True;
            intersect_return = [None, None, None];
            for i in range(i_start_index,len(self.edges)-1):
                is_i_intersection_free = True
                for j in range(i+1,len(self.edges)):

                    x1 = self.edges[i][0][1][0]
                    y1 = self.edges[i][0][1][1]
                    x1y1 = self.edges[i][0]
                    x2 = self.edges[i][1][1][0]
                    y2 = self.edges[i][1][1][1]
                    x2y2 = self.edges[i][1]
                    x3= self.edges[j][0][1][0]
                    y3= self.edges[j][0][1][1]
                    x3y3 = self.edges[j][0]
                    x4= self.edges[j][1][1][0]
                    y4 = self.edges[j][1][1][1]
                    x4y4= self.edges[j][1]

                    coordinates = [[x1,y1],[x2,y2]],[[x3,y3],[x4,y4]]

                    if  x1y1 == x3y3 or x1y1 == x4y4 or x2y2 == x3y3 or x2y2 == x4y4 :
                        pass
                    else:
                        intersect_return =find_intersect(coordinates)
                    intersect = intersect_return[0];


                    if intersect is True:
                        xcoor = intersect_return[1];
                        ycoor = intersect_return[2];
                        intersection_free = False;
                        is_i_intersection_free = False;
                    
                        # check if the intersection coordinates is in the vertex list, if not add it.
                        vertex_found = False;
                        
                        for k in self.vertex:
                            if k[1][0] == xcoor and k[1][1] == ycoor:
                                vertex_found = True;
                                intersection_pt = [ k[0],[xcoor,ycoor]]
                                break;
                        if vertex_found is False:
                            self.vertex_num = self.vertex_num + 1
                            self.vertex.append([self.vertex_num,[xcoor,ycoor]]);
                            intersection_pt = [self.vertex_num,[xcoor,ycoor]]

                        # We have to reconstruct the edges even if the vertex is already in list
                        del self.edges[j];
                        del self.edges[i];
                        
                        #let's reconstruct the edges with the newfound intersection point            
                        if (x1y1 != intersection_pt):
                            self.edges.append([intersection_pt,x1y1])
                        if (x2y2 != intersection_pt):
                            self.edges.append([intersection_pt,x2y2])
                        if (x3y3 != intersection_pt):
                            self.edges.append([intersection_pt,x3y3])
                        if (x4y4 != intersection_pt):
                            self.edges.append([intersection_pt,x4y4])  
                                             
                        break;
                
                if is_i_intersection_free == True:
                    i_start_index = i_start_index + 1;
                
                if intersection_free is False:
                    break;
        
         #  step 4: Remove the vertices and edges that doesn't meet the preconditions
        vertex_intersection_count_dict= {}
        nodes_to_be_removed = []
        for i in self.vertex:
            neighbour_nodes = []

            # Traverse the edge list and find the neibouring nodes
            for j in self.edges:
                if j[0] == i:
                    neighbour_nodes.append(j[1])
                elif j[1] == i:
                    neighbour_nodes.append(j[0])
                else:
                    continue
            
            vertex_intersection_count_dict[i[0]] = len(neighbour_nodes)      
            node_is_an_intersection = False;
            atleast_one_neighbour_valid = 0

            if len(neighbour_nodes) > 2:
                node_is_an_intersection = True;
            else:
                # Check if the neigbouring nodes are valid               
                for k in neighbour_nodes:
                    occ_count = 0
                    for p in self.edges:
                        if k == p[0]  or k == p[1]:
                            occ_count = occ_count + 1

                        if occ_count > 2:
                            atleast_one_neighbour_valid = 1
                            break;

            if atleast_one_neighbour_valid == False and node_is_an_intersection == False:
                nodes_to_be_removed.append(i)

        # remove the unwanteed nodes and edges
        edge_rem = [];
        for q in range(0,len(self.edges)):
            item = self.edges[q]
            if item[0] in  nodes_to_be_removed or item[1] in  nodes_to_be_removed :
                edge_rem.append(q);
            elif vertex_intersection_count_dict[item[0][0]] <= 2 and vertex_intersection_count_dict[item[1][0]] <=2:
                edge_rem.append(q);
            else:
                pass;
            
        while(edge_rem):
            del self.edges[edge_rem.pop()];

        for r in nodes_to_be_removed:
            self.vertex.remove(r)

        # Condition the output in the required format for a2 file input
        
        # flush the file first
        sys.stdout.flush()
        
        index_count = 0
        for k in self.vertex:
            index_count = index_count + 1
            k[0] = index_count
        print(str("V "+ str(index_count)),file=sys.stdout)

        print("E {",file=sys.stdout, end='')
        for i in range(len(self.edges)):
            item = self.edges[i];
            from_val = 0
            to_val = 0
            for k in self.vertex:
                if item[0][1] == k[1]:
                    from_val = k[0]
                if  item[1][1] == k[1]:
                    to_val = k[0]

            if(i == len(self.edges)-1):
                print (str("<"+str(from_val)+","+str(to_val)+">"),file=sys.stdout, end='')
            else:
                print (str("<"+str(from_val)+","+str(to_val)+">,"),file=sys.stdout, end='')
        print("}",file=sys.stdout)
                      
def main():

    Waterloo_city = Minimum_Traffic_Lights()
    while True:
        line = sys.stdin.readline()
        if line == "":
            break

        try:
            [command, street_name, raw_vertices] = parseLine(line)

            if command == 1:
                Waterloo_city.add(street_name,raw_vertices)
            elif command == 2:
                Waterloo_city.mod(street_name,raw_vertices)
            elif command == 3:
                Waterloo_city.rm(street_name)
            elif command == 4:
               Waterloo_city.gg()
            else:
                raise Exception('Unexpected error');
        except Exception as e:
            print('Error: [a1] ' + str(e), file=sys.stderr)
    # return exit code 0 on successful termination
    sys.exit(0)

if __name__ == "__main__":
    main()
